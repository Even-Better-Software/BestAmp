namespace BestAmpUI.Client.Models;

public class HostApisResponse : Response
{
    public const int OPCODE = 1003;

    public Dictionary<int, HostApiInfo> HostApis = [];

    public HostApisResponse(byte[] msg)
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

        while (start < msg.Length)
        {
            HostApiInfo info = new();

            int hostApiId = BitConverter.ToInt32(AsHostOrderBytes(msg, start, sizeof(int)));
            start += sizeof(int);

            info.Name = ParseAscii(msg[start ..]);
            start += STRING_LENGTH;
            
            info.DefaultInDevice = BitConverter.ToInt32(AsHostOrderBytes(msg, start, sizeof(int)));
            start += sizeof(int);

            info.DefaultOutDevice = BitConverter.ToInt32(AsHostOrderBytes(msg, start, sizeof(int)));
            start += sizeof(int);

            HostApis[hostApiId] = info;
        }
       
    }
}
