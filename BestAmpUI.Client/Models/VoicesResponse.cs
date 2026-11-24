namespace BestAmpUI.Client.Models;

public class VoicesResponse : Response
{
    public const int OPCODE = 1006;

    public Dictionary<int, string> VoicesIdToName = [];

    public VoicesResponse(byte[] msg)
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
            int voiceId = BitConverter.ToInt32(AsHostOrderBytes(msg, start, sizeof(int)));
            start += sizeof(int);

            string voiceName = ParseAscii(msg[start ..]);
            start += STRING_LENGTH;

            VoicesIdToName[voiceId] = voiceName;
        }
       
    }
}
