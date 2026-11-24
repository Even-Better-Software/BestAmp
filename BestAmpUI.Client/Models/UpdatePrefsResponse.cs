namespace BestAmpUI.Client.Models;

public class UpdatePrefsResponse : Response
{
    public const int OPCODE = 1001;

    public bool success = false;

    public UpdatePrefsResponse(byte[] msg)
    {
        int start = 0;

        int OpCode = BitConverter.ToInt32(AsHostOrderBytes(msg, start, sizeof(int)));
        start += sizeof(int);

        if (OpCode != OPCODE)
        {
            Console.WriteLine($"Error: Received msg w/ opcode {OpCode} but expected {OPCODE}");
            return;
        }

        int contentLength = BitConverter.ToInt32(AsHostOrderBytes(msg, start, sizeof(int)));
        start += sizeof(int);

        if (contentLength != (msg.Length - start))
        {
            Console.WriteLine($"Error: Mismatching content length.");
        }

        int content = BitConverter.ToInt32(AsHostOrderBytes(msg, start, sizeof(int)));
        success = content == 0;
    }
}
