import Foundation
import CoreBluetooth

// Nordic UART Service UUIDs
private let NUS_SERVICE_UUID = CBUUID(string: "6E400001-B5A3-F393-E0A9-E50E24DCCA9E")
private let NUS_RX_UUID      = CBUUID(string: "6E400002-B5A3-F393-E0A9-E50E24DCCA9E") // phone → watch

class BLEManager: NSObject, ObservableObject {
    @Published var isConnected   = false
    @Published var statusMessage = "Scanning..."

    private var central: CBCentralManager!
    private var watch:   CBPeripheral?
    private var rxChar:  CBCharacteristic?

    private var musicObserver: MusicObserver?

    override init() {
        super.init()
        // Restore state key lets CoreBluetooth reconnect automatically after app restart
        central = CBCentralManager(delegate: self, queue: nil,
                                   options: [CBCentralManagerOptionRestoreIdentifierKey: "SmartWatchCentral"])
    }

    // MARK: - Public write API

    func write(_ command: String) {
        guard let peripheral = watch,
              let characteristic = rxChar,
              let data = command.data(using: .utf8) else { return }
        // Watch firmware flushes on packet end, so no newline needed
        peripheral.writeValue(data, for: characteristic, type: .withResponse)
    }

    // MARK: - Time sync

    func sendTimeSync() {
        let now  = Date()
        let cal  = Calendar.current
        let h    = cal.component(.hour,       from: now)
        let m    = cal.component(.minute,     from: now)
        let s    = cal.component(.second,     from: now)
        let y    = cal.component(.year,       from: now)
        let mo   = cal.component(.month,      from: now)
        let d    = cal.component(.day,        from: now)
        // Calendar.weekday: 1=Sunday … 7=Saturday → convert to 0=Sunday … 6=Saturday
        let wday = cal.component(.weekday, from: now) - 1
        write(String(format: "T|%02d|%02d|%02d|%04d|%02d|%02d|%d", h, m, s, y, mo, d, wday))
    }
}

// MARK: - CBCentralManagerDelegate

extension BLEManager: CBCentralManagerDelegate {
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        if central.state == .poweredOn {
            // Scan specifically for NUS service (in scan response)
            central.scanForPeripherals(withServices: [NUS_SERVICE_UUID],
                                       options: [CBCentralManagerScanOptionAllowDuplicatesKey: false])
            DispatchQueue.main.async { self.statusMessage = "Scanning..." }
        }
    }

    func centralManager(_ central: CBCentralManager, willRestoreState dict: [String: Any]) {
        // Re-attach delegate to any peripherals that were connected before app restart
        if let peripherals = dict[CBCentralManagerRestoredStatePeripheralsKey] as? [CBPeripheral] {
            for p in peripherals where p.state == .connected {
                watch = p
                p.delegate = self
            }
        }
    }

    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral,
                        advertisementData: [String: Any], rssi RSSI: NSNumber) {
        guard peripheral.name == "SmartWatch" else { return }
        watch = peripheral
        central.stopScan()
        central.connect(peripheral, options: nil)
        DispatchQueue.main.async { self.statusMessage = "Connecting..." }
    }

    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        peripheral.delegate = self
        peripheral.discoverServices([NUS_SERVICE_UUID])
        DispatchQueue.main.async {
            self.isConnected   = true
            self.statusMessage = "Connected"
        }
    }

    func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral,
                        error: Error?) {
        rxChar        = nil
        musicObserver = nil
        DispatchQueue.main.async {
            self.isConnected   = false
            self.statusMessage = "Reconnecting..."
        }
        // Auto-reconnect
        central.connect(peripheral, options: nil)
    }

    func centralManager(_ central: CBCentralManager, didFailToConnect peripheral: CBPeripheral,
                        error: Error?) {
        watch = nil
        DispatchQueue.main.async { self.statusMessage = "Scanning..." }
        central.scanForPeripherals(withServices: [NUS_SERVICE_UUID])
    }
}

// MARK: - CBPeripheralDelegate

extension BLEManager: CBPeripheralDelegate {
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        guard let services = peripheral.services else { return }
        for service in services where service.uuid == NUS_SERVICE_UUID {
            peripheral.discoverCharacteristics([NUS_RX_UUID], for: service)
        }
    }

    func peripheral(_ peripheral: CBPeripheral,
                    didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        guard let chars = service.characteristics else { return }
        for char in chars where char.uuid == NUS_RX_UUID {
            rxChar = char
            // Immediately sync time on connect
            sendTimeSync()
            // Start forwarding music metadata
            musicObserver = MusicObserver(bleManager: self)
            musicObserver?.start()
        }
    }
}
