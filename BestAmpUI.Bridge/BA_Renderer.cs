using System.Threading;
using System.Runtime.InteropServices;
using Microsoft.Web.WebView2.WinForms;
using Microsoft.Web.WebView2.Core;


namespace BestAmpUI.Bridge.BA_Renderer;

public static class BA_Renderer
{
    // UI Customization
    private static readonly string WINDOW_NAME = "BestAmp";
    private static readonly int WINDOW_WIDTH = 1200;
    private static readonly int WINDOW_HEIGHT = 800;


    public static readonly Thread BA_UI_THREAD = new(() => {
        const int DWMWA_CAPTION_COLOR = 35;

        Form form = new()
        {
            Text = WINDOW_NAME,
            Width = WINDOW_WIDTH,
            Height = WINDOW_HEIGHT,
            BackColor = Color.Black,
        };

        WebView2 webView = new()
        {
            BackColor = Color.Black,
            Dock = DockStyle.Fill
        };

        form.Icon = Icon.ExtractAssociatedIcon(Application.ExecutablePath);
        form.Controls.Add(webView);

        form.Load += async (sender, e) =>
        {
            Color c = Color.Black;
            int colorRef = c.R | (c.G << 8) | (c.B << 16);
            DwmSetWindowAttribute(form.Handle, DWMWA_CAPTION_COLOR, ref colorRef, sizeof(int));

            try
            {
                string wwwroot = BA_Conf.BA_Conf.BA_AE_GUI;
                string ddir = BA_Conf.BA_Conf.BA_USER_DIR;

                Directory.CreateDirectory(ddir);
                CoreWebView2Environment env = await CoreWebView2Environment.CreateAsync(userDataFolder: ddir);
                await webView.EnsureCoreWebView2Async(env);
                webView.CoreWebView2.SetVirtualHostNameToFolderMapping(
                    "bestamp.test",
                    wwwroot,
                    CoreWebView2HostResourceAccessKind.Allow
                );
                webView.CoreWebView2.Navigate("http://bestamp.test/index.html");
            }
            catch (Exception ex)
            {
                MessageBox.Show($"WebView2 initialization failed: {ex.Message}");
            }
        };
        Application.Run(form);
    });



    [DllImport("dwmapi.dll", PreserveSig = true)]
    private static extern int DwmSetWindowAttribute(IntPtr hwnd, int attr, ref int attrValue, int attrSize);
}

