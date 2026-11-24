namespace BestAmpUI.Client.Models;
public class DeviceInfo
{
    public string Name { get; set; } = "";
    public int MaxInChannel { get; set; } = 0;
    public int MaxOutChannel { get; set; } = 0;
    public double SampleRate { get; set; } = 0;
    public override string ToString() => Name;
}
