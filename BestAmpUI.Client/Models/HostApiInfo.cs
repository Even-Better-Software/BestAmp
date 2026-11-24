namespace BestAmpUI.Client.Models;
public class HostApiInfo
{
    public string Name { get; set; } = "";
    public int DefaultInDevice { get; set; } = 0;
    public int DefaultOutDevice { get; set; } = 0;
    public override string ToString() => Name;
}