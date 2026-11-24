namespace BestAmpUI.Client.Models;

public class VoicesRequest : Request
{
    public const int OPCODE = 6;
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
