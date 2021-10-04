import Foundation

@_cdecl("ios_GetAssetDir")
public func ios_GetAssetDir() -> UnsafePointer<CChar>? {
    if let path = Bundle.main.path(forResource: "assets", ofType: "json")
    {
        let fileURL: URL = URL(fileURLWithPath: path)
        let folderURL = fileURL.deletingLastPathComponent()
        let dirString = folderURL.absoluteString.replacingOccurrences(of: "file://", with: "")
        return UnsafePointer<CChar>(dirString)
    }
    return UnsafePointer<CChar>("")
}
