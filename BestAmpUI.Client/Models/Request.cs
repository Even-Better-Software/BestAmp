namespace BestAmpUI.Client.Models;

public abstract class Request
{
    protected List<byte> Msg = [];

    /// <summary>
    /// Builds this message as a byte array.
    /// </summary>
    public byte[] BuildMsg()
    {
        Msg.Clear();

        AppendToMsg(BitConverter.GetBytes(GetOpCode()));
        AppendToMsg(BitConverter.GetBytes(GetContentLength()));
        BuildMsgContent();

        return [.. Msg];
    }

    /// <summary>
    /// Appends the message's payload. 
    /// This is called by BuildMsg() after the header has been appended.
    /// </summary>
    protected abstract void BuildMsgContent();

    /// <returns>
    /// The op code of this message.
    /// </returns>
    protected abstract int GetOpCode();

    /// <returns>
    /// The number of bytes in the content section of this message.
    /// </returns>
    protected abstract int GetContentLength();

    protected void AppendToMsg(byte[] bytes)
    {
        Msg.AddRange(BitConverter.IsLittleEndian ? bytes.Reverse() : bytes);
    }
}
