import Foundation
import UIKit

@_cdecl("ios_GetAssetDir")
public func ios_GetAssetDir() -> UnsafePointer<CChar>? {
    if let path = Bundle.main.path(forResource: "assets", ofType: "json") {
        let fileUrl = URL(fileURLWithPath: path)
        let folderUrl = fileUrl.deletingLastPathComponent()
        return UnsafePointer<CChar>(getFixedPath(path: folderUrl.path))
    }
    return UnsafePointer<CChar>("")
}

@_cdecl("ios_GetDisplayDensity")
public func ios_GetDisplayDensity() -> Float {
    return Float(UIScreen.main.scale)
}

@_cdecl("ios_GetSaveFile")
public func ios_GetSaveFile() -> UnsafePointer<CChar>? {
    do {
        let fileUrl = try FileManager.default
            .url(for: .applicationSupportDirectory, in: .userDomainMask, appropriateFor: nil, create: true)
            .appendingPathComponent("save.json")
        createSaveFile(filename: fileUrl)
        return UnsafePointer<CChar>(fileUrl.path)
    }
    catch {
        print("Error on ios_GetRWSaveFile: \(error)")
    }
    return UnsafePointer<CChar>("")
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
