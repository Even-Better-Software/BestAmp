// [WO] GUI message
namespace BestAmp.GUI.Models;

enum GUIMessageType
{
    Info,
    Error
}

class Message
{
    public string Content;
    public GUIMessageType GMType;

    public Message(string c, GUIMessageType t = GUIMessageType.Info)
    {
        this.Content = c;
        this.GMType = t;
    }
}
