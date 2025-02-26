using System.Text.Json;

namespace HostConsoleManage
{
    public class Message
    {
        public List<string> Messages { get; set; } = new List<string>();
        public List<string> Warnings { get; set; } = new List<string>();
        public List<string> Errors { get; set; } = new List<string>();

        public List<string> Output { get; set; } = new List<string>();
        public string? LastExitCode { get; set; } = "Unknown";

        public string SerializeToJson()
        {
            return JsonSerializer.Serialize(this);
        }
    }
}
