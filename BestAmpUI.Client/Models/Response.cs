using System.Text;

namespace BestAmpUI.Client.Models;

public abstract class Response
{
    protected const int STRING_LENGTH = 32;

    protected static Span<byte> AsHostOrderBytes(byte[] bytes, int spanStart, int spanLength)
    {
        Span<byte> span = bytes.AsSpan().Slice(spanStart, spanLength);
        if (BitConverter.IsLittleEndian)
        {
            span.Reverse();
        }
        
        return span;
    }

    protected static string ParseAscii(byte[] asciiString)
    {
        if (asciiString.Length < STRING_LENGTH)
        {
            throw new ArgumentException($"asciiString length must be > {STRING_LENGTH}");
        }

        StringBuilder builder = new();

        for (int i = 0; i < STRING_LENGTH; i++)
        {
            if (asciiString[i] == 0)
            {
                continue;
            }
            builder.Append((char)asciiString[i]);
        }

        if (asciiString[STRING_LENGTH - 2] != '\0')
        {
            builder.Append("...");
        }

        return builder.ToString();
    }

    public static int OpcodeFromMessage(byte[] msg)
    {
        byte[] bytes = msg[0 .. sizeof(int)];
        if (BitConverter.IsLittleEndian)
        {
            bytes = [.. bytes.Reverse()];
        }

        return BitConverter.ToInt32(bytes);
    }
}
