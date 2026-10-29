#[allow(non_camel_case_types)]
#[allow(non_snake_case)]
#[allow(dead_code)]
mod win32;

use win32::*;
use windows_core::*;

#[allow(non_snake_case)]
extern "system" fn WndProc(hWnd: HWND, msg: u32, wParam: WPARAM, lParam: LPARAM) -> LRESULT {
    unsafe {
        static mut COUNT: i32 = 0;
        match msg as i32 {
            WM_CREATE => {
                COUNT += 1;
                return LRESULT(0);
            }
            WM_DESTROY => {
                COUNT -= 1;
                if COUNT == 0 {
                    PostQuitMessage(0);
                }
                return LRESULT(0);
            }
            _ => DefWindowProcW(hWnd, msg, wParam, lParam),
        }
    }
}

fn create_window(title: &str, width: i32, height: i32) -> HWND {
    unsafe {
        let title = HSTRING::from(title);
        let class = HSTRING::from("D3D11Application");
        let title = PCWSTR::from_raw(title.as_ptr());
        let class = PCWSTR::from_raw(class.as_ptr());
        let wc = WNDCLASSW {
            style: (CS_HREDRAW | CS_VREDRAW) as _,
            lpszClassName: class,
            lpfnWndProc: Some(WndProc),
            hInstance: GetModuleHandleW(PCWSTR::null()),
            hCursor: LoadCursorW(None, IDC_ARROW),
            hbrBackground: HBRUSH(GetStockObject(BLACK_BRUSH).0),
            lpszMenuName: PCWSTR::null(),
            hIcon: HICON(0 as _),
            cbClsExtra: 0,
            cbWndExtra: 0,
        };

        // fails on second call to create_window(). this is intended,
        // as an WM_DESTROY message to any window ends the mainloop,
        // and thus ends the program. checking if all windows are
        // closed in the mainloop is hard, so there is no multi-
        // window support.
        if RegisterClassW(&wc) == ATOM(0) {
            panic!("failed to register window class, error {}", GetLastError());
        }

        let mut rect = RECT {
            left: 0,
            top: 0,
            right: width,
            bottom: height,
        };
        let style = (WS_OVERLAPPEDWINDOW & !(WS_SIZEBOX | WS_MAXIMIZEBOX)) as u32;
        _ = AdjustWindowRect(&mut rect, style, false);

        let width = rect.right - rect.left;
        let height = rect.bottom - rect.top;
        let x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
        let y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

        let hwnd = CreateWindowExW(
            0,
            class,
            title,
            style,
            x,
            y,
            width,
            height,
            None,
            None,
            Some(wc.hInstance),
            None,
        );

        if hwnd.0 as u32 == 0 {
            panic!("failed to create window, error {}", GetLastError());
        }

        _ = ShowWindow(hwnd, SW_SHOW);
        _ = UpdateWindow(hwnd);

        hwnd
    }
}

trait HresultExtensions {
    fn throw_if_failed(&self);
    fn succeed(&self) -> bool;
    fn failed(&self) -> bool;
}

impl HresultExtensions for HRESULT {
    fn throw_if_failed(&self) {
        if self.failed() {
            panic!("operation failed, HRESULT = {}", self.0);
        }
    }

    fn succeed(&self) -> bool {
        self.0 == 0
    }

    fn failed(&self) -> bool {
        self.0 != 0
    }
}

struct Context {
    hwnd: HWND,
    device: ID3D11Device,
    context: ID3D11DeviceContext,
}

impl Context {
    fn new(hwnd: HWND) -> Self {
        unsafe {
            let driver_types = [
                D3D_DRIVER_TYPE_HARDWARE,
                D3D_DRIVER_TYPE_WARP,
                D3D_DRIVER_TYPE_REFERENCE,
            ];
            let feature_levels = [D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0];
            let mut device: Option<ID3D11Device> = None;
            let mut context: Option<ID3D11DeviceContext> = None;
            let mut hr: HRESULT = HRESULT(0);

            for driver_type in driver_types {
                let mut i = 0;
                while i < feature_levels.len() {
                    hr = D3D11CreateDevice(
                        None,
                        driver_type,
                        HINSTANCE(0 as _),
                        if cfg!(debug_assertions) {
                            D3D11_CREATE_DEVICE_DEBUG
                        } else {
                            0
                        } as _,
                        Some(&feature_levels[i..]),
                        D3D11_SDK_VERSION as _,
                        Some(&mut device),
                        None,
                        Some(&mut context),
                    );
                    if hr.succeed() {
                        break;
                    }
                    i += 1;
                }
                if hr.succeed() {
                    break;
                }
            }

            hr.throw_if_failed();
            let device = device.unwrap();
            let context = context.unwrap();

            let level = device
                .CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4)
                .unwrap();

            let dxgi_device: IDXGIDevice = device.cast().unwrap();
            let adapter: IDXGIAdapter = dxgi_device.GetAdapter().unwrap();
            let factory: IDXGIFactory1 = adapter.GetParent().unwrap();

            CoInitializeEx(None, COINIT_MULTITHREADED as _).throw_if_failed();

            Context {
                hwnd: hwnd,
                device: device,
                context: context,
            }
        }
    }
}

trait Scene {
    fn init(&mut self, ctx: &mut Context);
    fn update(&mut self, ctx: &mut Context);
    fn draw(&mut self, ctx: &mut Context);
}

fn mainloop(title: &str, width: i32, height: i32, main_scene: &mut dyn Scene) {
    unsafe {
        let hwnd = create_window(title, width, height);
        let mut ctx = Context::new(hwnd);
        let mut msg = MSG::default();

        main_scene.init(&mut ctx);

        while msg.message != WM_QUIT as _ {
            if PeekMessageW(&mut msg, None, 0, 0, PM_REMOVE as _).as_bool() {
                _ = TranslateMessage(&msg);
                DispatchMessageW(&msg);
            } else {
                main_scene.update(&mut ctx);
                main_scene.draw(&mut ctx);
            }
        }
    }
}

struct TestScene;

impl Scene for TestScene {
    fn init(&mut self, _ctx: &mut Context) {}
    fn update(&mut self, _ctx: &mut Context) {}
    fn draw(&mut self, _ctx: &mut Context) {}
}

fn main() {
    let mut scene = TestScene;
    mainloop("La Tale Doujin", 1360, 768, &mut scene);
}
