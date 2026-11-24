namespace BestAmpUI.Client.Models;

public class HostApisRequest : Request
{
    public const int OPCODE = 3;
    private const int CONTENT_LENGTH = 0;

    protected override void BuildMsgContent() { }

    protected override int GetContentLength()
    {
        return CONTENT_LENGTH;
    }

    protected override int GetOpCode()
    {
        return OPCODE;
    }
}
