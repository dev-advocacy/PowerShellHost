using System.Management.Automation;
using System.Management.Automation.Runspaces;
using System.Xml.Linq;

namespace HostLibrary
{
    public class HostedRunspace
    {
        /// <summary>
        /// The PowerShell runspace pool.
        /// </summary>
        private RunspacePool? RsPool { get; set; }

        /// <summary>
        /// Initialize the runspace pool.
        /// </summary>
        /// <param name="minRunspaces"></param>
        /// <param name="maxRunspaces"></param>
        public void InitializeRunspaces(int minRunspaces, int maxRunspaces, string[] modulesToLoad)
        {
            // create the default session state.
            // session state can be used to set things like execution policy, language constraints, etc.
            // optionally load any modules (by name) that were supplied.

            try
            {
                var defaultSessionState = InitialSessionState.CreateDefault();

                // Test if this is Linux or MacOS
                if (Environment.OSVersion.Platform == PlatformID.Win32NT)
                {
                    defaultSessionState.ExecutionPolicy = Microsoft.PowerShell.ExecutionPolicy.Unrestricted;
                }

                foreach (var moduleName in modulesToLoad)
                {
                    defaultSessionState.ImportPSModule(moduleName);
                }

                // use the runspace factory to create a pool of runspaces
                // with a minimum and maximum number of runspaces to maintain.

                RsPool = RunspaceFactory.CreateRunspacePool(defaultSessionState);
                RsPool.SetMinRunspaces(minRunspaces);
                RsPool.SetMaxRunspaces(maxRunspaces);

                // set the pool options for thread use.
                // we can throw away or re-use the threads depending on the usage scenario.

                RsPool.ThreadOptions = PSThreadOptions.UseNewThread;

                // open the pool. 
                // this will start by initializing the minimum number of runspaces.

                RsPool.Open();
            }
            catch (Exception ex)
            {
                Console.WriteLine($"exception {ex}");
            }
        }

        /// <summary>
        /// Runs a PowerShell script with parameters and prints the resulting pipeline objects to the console output. 
        /// </summary>
        /// <param name="scriptContents">The script file contents.</param>
        /// <param name="scriptParameters">A dictionary of parameter names and parameter values.</param>
        public async Task RunScript(string scriptContents, Dictionary<string, object> scriptParameters)
        {
            if (RsPool == null)
            {
                throw new ApplicationException("Runspace Pool must be initialized before calling RunScript().");
            }

            try
            {

                using (PowerShell ps = PowerShell.Create())
                {
                    // use the runspace pool.
                    ps.RunspacePool = RsPool;
                    ps.AddScript(scriptContents);

                    var dd = ps.Connect();
                    

                    if (scriptParameters != null)
                    {
                        foreach (var param in scriptParameters)
                        {
                            ps.AddParameter(param.Key, param.Value);
                        }
                    }

                    ps.Streams.Error.DataAdded += Error_DataAdded;
                    ps.Streams.Warning.DataAdded += Warning_DataAdded;
                    ps.Streams.Information.DataAdded += Information_DataAdded;



                    //var results = ps.Invoke();
                    //// specify the script code to run.
                    //ps.AddScript(scriptContents);

                    //// specify the parameters to pass into the script.
                    //if (scriptParameters != null)
                    //    ps.AddParameters(scriptParameters);



                    //var res = ps
                    //    .AddScript(scriptContents)
                    //    //.AddArgument("name")
                    //    //.AddArgument("12")
                    //    .Invoke<string>();
                    

                    // subscribe to events from some of the streams
                   

                    // execute the script and await the result.
                    var pipelineObjects = await ps.InvokeAsync().ConfigureAwait(false);

                    // print the resulting pipeline objects to the console.
                    Console.WriteLine("----- Pipeline Output below this point -----");
                    foreach (var item in pipelineObjects)
                    {
                        Console.WriteLine(item.BaseObject.ToString());
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"exception {ex}");
            }
        }

        /// <summary>
        /// Handles data-added events for the information stream.
        /// </summary>
        /// <remarks>
        /// Note: Write-Host and Write-Information messages will end up in the information stream.
        /// </remarks>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Information_DataAdded(object? sender, DataAddedEventArgs e)
        {
            var streamObjectsReceived = sender as PSDataCollection<InformationRecord>;
            if (streamObjectsReceived == null)
            {
                return;
            }
            var currentStreamRecord = streamObjectsReceived[e.Index];
            Console.WriteLine($"InfoStreamEvent: {currentStreamRecord.MessageData}");
        }

        /// <summary>
        /// Handles data-added events for the warning stream.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Warning_DataAdded(object? sender, DataAddedEventArgs e)
        {
            var streamObjectsReceived = sender as PSDataCollection<WarningRecord>;
            if (streamObjectsReceived == null)
            {
                return;
            }
            var currentStreamRecord = streamObjectsReceived[e.Index];
            Console.WriteLine($"WarningStreamEvent: {currentStreamRecord.Message}");
        }

        /// <summary>
        /// Handles data-added events for the error stream.
        /// </summary>
        /// <remarks>
        /// Note: Uncaught terminating errors will stop the pipeline completely.
        /// Non-terminating errors will be written to this stream and execution will continue.
        /// </remarks>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Error_DataAdded(object? sender, DataAddedEventArgs e)
        {
            var streamObjectsReceived = sender as PSDataCollection<ErrorRecord>;
            if (streamObjectsReceived == null)
            {
                return;
            }
            var currentStreamRecord = streamObjectsReceived[e.Index];
            Console.WriteLine($"ErrorStreamEvent: {currentStreamRecord.Exception}");
        }
    }
}
