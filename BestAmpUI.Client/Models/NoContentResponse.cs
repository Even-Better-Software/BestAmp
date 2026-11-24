namespace BestAmpUI.Client.Models;

public abstract class NoContentResponse : Response
{
    protected abstract int GetOpCode();

    public NoContentResponse(byte[] msg)
    {
        int start = 0;

        int OpCode = BitConverter.ToInt32(AsHostOrderBytes(msg, start, sizeof(int)));
        start += sizeof(int);

        if (OpCode != GetOpCode())
        {
            Console.WriteLine($"Error: Received msg w/ opcode {OpCode} but expected {GetOpCode()}");
            return;
        }

        int contentLength = BitConverter.ToInt32(AsHostOrderBytes(msg, start, sizeof(int)));
        start += sizeof(int);

        if (contentLength != (msg.Length - start))
        {
            Console.WriteLine($"Error: Mismatching content length.");
        }
    }
}

public class UpdateAmpValuesResponse(byte[] msg) : NoContentResponse(msg)
{
    public const int OPCODE = 1002;

    protected override int GetOpCode() => OPCODE;
}

public class OkResponse(byte[] msg) : NoContentResponse(msg)
{
    public const int OPCODE = 9998;
    protected override int GetOpCode() => OPCODE;
}

public class BadResponse(byte[] msg) : NoContentResponse(msg)
{
    public const int OPCODE = 9999;
    protected override int GetOpCode() => OPCODE;
}
