namespace BestAmpUI.Client.Models;

public class UpdatePrefsRequest : Request
{
    private const int OPCODE = 1;
    private const int CONTENT_LENGTH = 7 * sizeof(int) + 1 * sizeof(double);
    public int HostApi { get; set; } = 0;
    public int DeviceIn { get; set; } = 0;
    public int ChannelIn { get; set; } = 0;
    public int DeviceOut { get; set; } = 0;
    public int ChannelOut { get; set; } = 0;
    public double SampleRate { get; set; } = 0;
    public int FramesPerBuffer { get; set; } = 0;
    public int SampleFormat { get; set; } = 0;

    protected override void BuildMsgContent()
    {
        AppendToMsg(BitConverter.GetBytes(HostApi));
        AppendToMsg(BitConverter.GetBytes(DeviceIn));
        AppendToMsg(BitConverter.GetBytes(DeviceOut));
        AppendToMsg(BitConverter.GetBytes(ChannelIn));
        AppendToMsg(BitConverter.GetBytes(ChannelOut));
        AppendToMsg(BitConverter.GetBytes(FramesPerBuffer));
        AppendToMsg(BitConverter.GetBytes(SampleRate));
        AppendToMsg(BitConverter.GetBytes(SampleFormat));
    }

    protected override int GetContentLength()
    {
        return CONTENT_LENGTH;
    }

    protected override int GetOpCode()
    {
        return OPCODE;
    }
}
