namespace BestAmpUI.Client.Models;

public class UpdateAmpValuesRequest : Request
{
    private const int OPCODE = 2;
    private const int CONTENT_LENGTH = 5 * sizeof(int);
    public int Voice { get; set; } = 0;
    public int Gain { get; set; } = 10;
    public int Volume { get; set; } = 10;
    public int Bass { get; set; } = 10;
    public int Treble { get; set; } = 10;

    protected override void BuildMsgContent()
    {
        AppendToMsg(BitConverter.GetBytes(Gain));
        AppendToMsg(BitConverter.GetBytes(Bass));
        AppendToMsg(BitConverter.GetBytes(Treble));
        AppendToMsg(BitConverter.GetBytes(Volume));
        AppendToMsg(BitConverter.GetBytes(Voice));
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
