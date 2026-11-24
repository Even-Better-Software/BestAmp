namespace BestAmpUI.Client.Models;

public class DeviceIdsRequest : Request
{
    public const int OPCODE = 5;
    private const int CONTENT_LENGTH = sizeof(int);
    public int HostApi { get; set; } = 0;

    protected override void BuildMsgContent()
    {
        AppendToMsg(BitConverter.GetBytes(HostApi));
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
