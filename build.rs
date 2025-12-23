fn main() {
    if std::env::var("CARGO_CFG_TARGET_OS").unwrap() == "windows" {
        let mut res = winres::WindowsResource::new();
        // Point to the icon file that will be generated in the root or assets folder
        // We will ensure 'assets/icon.ico' exists before build
        res.set_icon("assets/icon.ico");
        res.compile().unwrap();
    }
}
