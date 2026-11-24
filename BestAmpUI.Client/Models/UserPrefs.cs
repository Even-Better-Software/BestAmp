namespace BestAmpUI.Client.Models;

public class UserPrefs
{
    public const string DEFAULT = "Default"; 
    /*
    DEV NOTE: the request classes happen to hold the same values that we want
    the user to be able to save, so I'll just compose the UserPrefs class out of them.
    */  

    public UpdatePrefsRequest? Settings { get; set; }
    public string CurrentProfile { get; set; } = DEFAULT; 
    public Dictionary<string, UpdateAmpValuesRequest> Profiles { get; set; } = new() {
        { DEFAULT, new UpdateAmpValuesRequest() },
    };
}
