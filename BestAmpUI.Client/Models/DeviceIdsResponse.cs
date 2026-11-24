namespace BestAmpUI.Client.Models;

public class DeviceIdsResponse : Response
{
    public const int OPCODE = 1005;

    public Dictionary<int, DeviceInfo> Devices = [];

    public DeviceIdsResponse(byte[] msg)
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
            DeviceInfo device = new();

            int deviceId = BitConverter.ToInt32(AsHostOrderBytes(msg, start, sizeof(int)));
            start += sizeof(int);

            device.Name = ParseAscii(msg[start ..]);
            start += STRING_LENGTH;
            
            device.MaxInChannel = BitConverter.ToInt32(AsHostOrderBytes(msg, start, sizeof(int)));
            start += sizeof(int);

            device.MaxOutChannel = BitConverter.ToInt32(AsHostOrderBytes(msg, start, sizeof(int)));
            start += sizeof(int);

            device.SampleRate = BitConverter.ToDouble(AsHostOrderBytes(msg, start, sizeof(double)));
            start += sizeof(double);

            Devices[deviceId] = device;
        }
       
    }
}
