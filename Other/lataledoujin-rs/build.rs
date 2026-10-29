fn main() {
    windows_bindgen::builder()
        .output("src/win32.rs")
        .filter_file("NativeMethods.txt")
        .flat()
        .write();
}
