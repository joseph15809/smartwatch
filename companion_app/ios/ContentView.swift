import SwiftUI

struct ContentView: View {
    @EnvironmentObject var ble: BLEManager

    var body: some View {
        VStack(spacing: 24) {
            Image(systemName: ble.isConnected ? "applewatch.radiowaves.left.and.right" : "applewatch")
                .font(.system(size: 64))
                .foregroundColor(ble.isConnected ? .green : .secondary)

            Text("SmartWatch Companion")
                .font(.title2).bold()

            Text(ble.statusMessage)
                .foregroundColor(.secondary)

            if ble.isConnected {
                Button("Sync Time") {
                    ble.sendTimeSync()
                }
                .buttonStyle(.borderedProminent)
            }
        }
        .padding()
    }
}
