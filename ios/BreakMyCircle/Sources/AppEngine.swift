import Foundation
import UIKit

let C_STRING_EMPTY = Utility.makeCString(from: "")
var C_STRING_ASSET_DIR: UnsafeMutablePointer<Int8>? = nil
var C_STRING_SAVE_FILE: UnsafeMutablePointer<Int8>? = nil

@_cdecl("ios_GetAssetDir")
public func ios_GetAssetDir() -> UnsafeMutablePointer<Int8> {
    if C_STRING_ASSET_DIR == nil {
        C_STRING_ASSET_DIR = C_STRING_EMPTY
        if let path = Bundle.main.path(forResource: "assets", ofType: "json") {
            let fileUrl = URL(fileURLWithPath: path)
            let folderUrl = fileUrl.deletingLastPathComponent()
            C_STRING_ASSET_DIR = Utility.makeCString(from: getFixedPath(path: folderUrl.path))
        }
    }
    return C_STRING_ASSET_DIR!
}

@_cdecl("ios_GetCanvasVerticalPadding")
public func ios_GetCanvasVerticalPadding() -> Float {
    var statusBarHeight: CGFloat = 0
    if #available(iOS 13.0, *) {
        let window = UIApplication.shared.windows.filter {$0.isKeyWindow}.first
        statusBarHeight = window?.windowScene?.statusBarManager?.statusBarFrame.height ?? 0
    }
    else {
        statusBarHeight = UIApplication.shared.statusBarFrame.height
    }
    return Float(statusBarHeight * UIScreen.main.scale) // Get in native pixels, not scaled ones
}

@_cdecl("ios_GetDisplayDensity")
public func ios_GetDisplayDensity() -> Float {
    return Float(UIScreen.main.scale)
}

@_cdecl("ios_GetSaveFile")
public func ios_GetSaveFile() -> UnsafeMutablePointer<Int8> {
    if C_STRING_SAVE_FILE == nil {
        C_STRING_SAVE_FILE = C_STRING_EMPTY
        do {
            let fileUrl = try FileManager.default
                .url(for: .applicationSupportDirectory, in: .userDomainMask, appropriateFor: nil, create: true)
                .appendingPathComponent("save.json")
            let _ = createSaveFile(filename: fileUrl)
            C_STRING_SAVE_FILE = Utility.makeCString(from: fileUrl.path)
        }
        catch {
            print("Error on ios_GetRWSaveFile: \(error)")
        }
    }
    return C_STRING_SAVE_FILE!
}

private func getFixedPath(path: String) -> String {
    return path.hasSuffix("/") ? path : (path + "/")
}

private func createSaveFile(filename: URL) -> Bool {
    if FileManager.default.fileExists(atPath: filename.path) {
        return true
    }
    else {
        let data = "{}".data(using: .utf8)
        do {
            try data?.write(to: filename)
            return true
        } catch {
            print("Error on createSaveFile: \(error)")
        }
    }
    return false
}
