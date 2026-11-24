using System.Runtime.InteropServices;
using Microsoft.Web.WebView2.WinForms;
using Microsoft.Web.WebView2.Core;

namespace BestAmpUI.Bridge;

public static class Renderer
{
    // UI Customization
    private const string WINDOW_NAME = "BestAmp";
    private const int WINDOW_WIDTH = 1200;
    private const int WINDOW_HEIGHT = 800;
    private const int DWMWA_CAPTION_COLOR = 35;

    public static readonly Thread UIThread = new(() => {
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
                Directory.CreateDirectory(Config.DataDir);
                CoreWebView2Environment env = await CoreWebView2Environment.CreateAsync(
                    null, Config.DataDir, null
                );
                await webView.EnsureCoreWebView2Async(env);
                webView.CoreWebView2.SetVirtualHostNameToFolderMapping(
                    Config.VHOST_NAME,
                    Config.WwwrootPath,
                    CoreWebView2HostResourceAccessKind.Allow
                );
                webView.CoreWebView2.Navigate($"http://{Config.VHOST_NAME}/index.html");
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

