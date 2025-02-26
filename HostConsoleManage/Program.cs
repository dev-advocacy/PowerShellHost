using CommandLine;
using HostConsoleManage;
using System.Text;

internal class Program
{
    public class CommandLineOptions
    {
        [Option('f', "File", Required = true, HelpText = "File to execute")]
        public string? File { get; set; }
    }

    static void Main(string[] args)
    {
        Parser.Default.ParseArguments<CommandLineOptions>(args).WithParsed<CommandLineOptions>(o =>
        {
            Console.WriteLine(o.File);
        });

        Parser.Default.ParseArguments<CommandLineOptions>(args).WithParsed<CommandLineOptions>(async o =>
        {
            if (o.File == null)
            {
                Console.WriteLine("Please provide a file to execute");
                return;
            }

            if (!System.IO.File.Exists(o.File))
            {
                Console.WriteLine("File does not exist");
                return;
            }
            //we do not loads modules in this example
            var scriptContents = new StringBuilder();
            using (StreamReader sr = new StreamReader(o.File))
            {
                scriptContents.Append(sr.ReadToEnd());
            }

            try
            {
                if (scriptContents != null)
                {
                    RunPowershellScript runPowershellScript = new RunPowershellScript();
                    runPowershellScript.InitializePowerShell(Microsoft.PowerShell.ExecutionPolicy.Unrestricted);
                    var ret = runPowershellScript.ExecutePowerShellScript(scriptContents.ToString());
                    Console.WriteLine(ret);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"exception {ex}");
            }
        });
        Console.ReadLine();
    }
}