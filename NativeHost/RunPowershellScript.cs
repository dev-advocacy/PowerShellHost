using System;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq.Expressions;
using System.Management.Automation;
using System.Management.Automation.Language;
using System.Management.Automation.Runspaces;
using System.Reflection.Metadata;
using System.Text.RegularExpressions;
using static System.Runtime.InteropServices.JavaScript.JSType;

namespace HostConsoleManage
{
    public class RunPowershellScript 
    {
        private InitialSessionState? _sessionState = null;
        private PowerShell? _powerShellInstance = null;
        private string?[]? Messages
        {
            get;
            set;
        }
        private string[]? Warnings
        {
            get;
            set;
        }
        private string[]? Verbose
        {
            get;
            set;
        }

        private string[]? Errors
        {
            get;
            set;
        }
        private string[]? Progress
        {
            get;
            set;
        }

        private string? ExitCode
        {
            get;
            set;
        }
        private string? lastErrorCode
        {
            get;
            set;
        }
        private Message message = new Message();

        private readonly Regex errorInFileRegex = new Regex(@"{ERROR:([^\|]+)\|([^\|]+)\|([^\|]+)\|([^\|]+)}", RegexOptions.Compiled | RegexOptions.IgnoreCase);



        /// <summary>
        /// Initialize the PowerShell instance.
        /// </summary>
        /// <param name="policy"></param>
        /// <returns></returns>
        public void InitializePowerShell(Microsoft.PowerShell.ExecutionPolicy policy)
        {
            _sessionState = InitialSessionState.CreateDefault();
            _sessionState.ExecutionPolicy = policy;
            _powerShellInstance = PowerShell.Create(_sessionState);
        }

        private List<string>? DecodeOutput(Collection<PSObject> output)
        {
            List<string> outputlist = new List<string>();
            foreach (var record in output)
            {
                if (record.BaseObject is ErrorRecord errorRecord)
                {
                    Debug.WriteLine($"ErrorStreamEvent: {errorRecord.Exception.Message}");
                }
                else if (record.BaseObject is WarningRecord warningRecord)
                {
                    Debug.WriteLine($"WarningStreamEvent: {warningRecord.Message}");
                }
                else if (record.BaseObject is InformationRecord informationRecord)
                {
                    Debug.WriteLine($"InfoStreamEvent: {informationRecord.MessageData}");
                }
                else
                {
                    var value = record.TypeNames.FirstOrDefault(type => type == "System.String");
                    if (value != null)
                    {
                        outputlist.Add(record.ToString());
                    }
                }
            }
            return outputlist;
        }

        /// <summary>
        /// Execute the powershell script using filename and parameters.
        /// </summary>
        /// <param name="expression"></param>
        /// <param name="parameter"></param>
        /// <returns></returns>
        public string ExecutePowerShellFile(string? expression, List<string>? parameter)
        {
            if (_powerShellInstance != null)
            {
                try
                {
                    if (parameter != null && parameter.Count > 0)
                    {
                        expression += " " + string.Join("`\"", parameter);
                    }

                    _powerShellInstance.AddScript("Invoke-Expression \"" + expression + "\"", true /* useLocalScope */);
                    _powerShellInstance.Streams.Information.DataAdded += OnInformationDataAdded;
                    _powerShellInstance.Streams.Warning.DataAdded += OnWarningDataAdded;
                    _powerShellInstance.Streams.Error.DataAdded += OnErrorDataAdded;
                    _powerShellInstance.Streams.Progress.DataAdded += Progress_DataAdded;
                    _powerShellInstance.Streams.Verbose.DataAdded += Verbose_DataAdded;

                    var output = _powerShellInstance.Invoke<PSObject>();

                    var outputlist = DecodeOutput(output);

                    Messages = _powerShellInstance.Streams.Information.Select(record => record.MessageData.ToString()).ToArray();
                    Warnings = _powerShellInstance.Streams.Warning.Select(record => record.ToString()).ToArray();
                    Errors = _powerShellInstance.Streams.Error.Select(record => record.ToString()).ToArray();
                    Progress = _powerShellInstance.Streams.Progress.Select(record => record.ToString()).ToArray();
                    Verbose = _powerShellInstance.Streams.Verbose.Select(record => record.ToString()).ToArray();
                    var tt = _powerShellInstance.Runspace;
                    var ExitCode = _powerShellInstance.Runspace.SessionStateProxy.GetVariable("LASTEXITCODE");

                    if (ExitCode != null)
                    {
                        message.LastExitCode = ExitCode.ToString();
                    }
                    if (ExitCode == null && lastErrorCode != null)
                    {
                        message.LastExitCode = lastErrorCode;
                    }
                    else
                    {
                        message.LastExitCode = "0";
                    }
                    if (Messages != null) message.Messages = Messages.ToList()!;
                    if (Warnings != null) message.Warnings = Warnings.ToList();
                    if (Errors != null) message.Errors = Errors.ToList();
                    if (outputlist != null) message.Output = outputlist;
                }
                catch (System.Management.Automation.RuntimeException parseEx)
                {
                    message.Errors.Add(parseEx.Message);
                }
            }
            else
            {
                message.Errors.Add("Powershell Instance is null");
            }
            return message.SerializeToJson();
        }

        

        public string ExecutePowerShellScript(string? script)
        {
            if (_powerShellInstance != null)
            {
                try
                {
                    _powerShellInstance.AddScript(script);
                    _powerShellInstance.Streams.Information.DataAdded += OnInformationDataAdded;
                    _powerShellInstance.Streams.Warning.DataAdded += OnWarningDataAdded;
                    _powerShellInstance.Streams.Error.DataAdded += OnErrorDataAdded;
                    _powerShellInstance.Streams.Progress.DataAdded += Progress_DataAdded;
                    _powerShellInstance.Streams.Verbose.DataAdded += Verbose_DataAdded;

                    var output = _powerShellInstance.Invoke<PSObject>();

                    var outputlist = DecodeOutput(output);

                    if (_powerShellInstance != null)
                    {
                        Messages = _powerShellInstance.Streams.Information.Select(record => record.MessageData.ToString()).ToArray();
                        Warnings = _powerShellInstance.Streams.Warning.Select(record => record.ToString()).ToArray();
                        Errors = _powerShellInstance.Streams.Error.Select(record => record.ToString()).ToArray();
                        Progress = _powerShellInstance.Streams.Progress.Select(record => record.ToString()).ToArray();
                        Verbose = _powerShellInstance.Streams.Verbose.Select(record => record.ToString()).ToArray();
                        var tt = _powerShellInstance.Runspace;
                        var ExitCode = _powerShellInstance.Runspace.SessionStateProxy.GetVariable("LASTEXITCODE");

                        if (ExitCode != null)
                        {
                            message.LastExitCode = ExitCode.ToString();
                        }

                        if (ExitCode == null && lastErrorCode != null)
                        {
                            message.LastExitCode = lastErrorCode;
                        }
                        else
                        {
                            message.LastExitCode = "0";
                        }
                    }

                    if (Messages != null) message.Messages = Messages.ToList()!;
                    if (Warnings != null) message.Warnings = Warnings.ToList();
                    if (Errors != null) message.Errors = Errors.ToList();
                    if (outputlist != null) message.Output = outputlist;

                }
                catch (System.Management.Automation.ParseException parseEx)
                {
                    message.Errors.Add(parseEx.Message);
                }
            }
            else
            {
                message.Errors.Add("Powershell Instance is null");
            }            
            return message.SerializeToJson();
        }

        /// <summary>
        /// Get the new record from the data added event.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        /// <returns></returns>
        private T GetNewRecord<T>(object sender, DataAddedEventArgs e)
        {
            return ((PSDataCollection<T>)sender)[e.Index];
        }
        /// <summary>
        /// Event handler for information data added to the information stream.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnInformationDataAdded(object? sender, DataAddedEventArgs e)
        {
            if (sender != null)
            {
                var information = GetNewRecord<InformationRecord>(sender, e);               
                var messageData = information.MessageData.ToString();

                if (!string.IsNullOrWhiteSpace(messageData))
                {
                    MatchCollection matches = errorInFileRegex.Matches(messageData);

                    if (matches.Count > 0)
                    {
                        foreach (Match match in matches)
                        {
                            if (match.Groups.Count >= 5)
                            {
                                Debug.WriteLine(System.Diagnostics.Debugger.IsAttached ? $"ErrorStreamEvent: {messageData}" : $"ErrorStreamEvent: {match.Groups[1].Value}");
                            }
                        }
                    }
                    else
                    {
                        Debug.WriteLine($"InfoStreamEvent: {messageData}");
                    
                    }
                }
            }
        }
        /// <summary>
        /// Event handler for warning data added to the warning stream.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnWarningDataAdded(object? sender, DataAddedEventArgs e)
        {
            if (sender != null)
            {
                var warning = GetNewRecord<WarningRecord>(sender, e);
                Debug.WriteLine(System.Diagnostics.Debugger.IsAttached ? $"WarningStreamEvent: {warning.ToString()}" : $"WarningStreamEvent: {warning.Message}");
            }
        }
        /// <summary>
        /// Event handler for error data added to the error stream.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnErrorDataAdded(object? sender, DataAddedEventArgs e)
        {
            if (sender != null)
            {
                var error = GetNewRecord<ErrorRecord>(sender, e);
                lastErrorCode = error.Exception.HResult.ToString();
                Debug.WriteLine(System.Diagnostics.Debugger.IsAttached ? $"ErrorStreamEvent: {error.ToString()}" : $"ErrorStreamEvent: {error.Exception.Message}");
            }
        }
        private void Progress_DataAdded(object? sender, DataAddedEventArgs e)
        {
            if (sender != null)
            {
                
                var progress = GetNewRecord<ProgressRecord>(sender, e);
                Debug.WriteLine(System.Diagnostics.Debugger.IsAttached ? "" : $"CurrentOperation: {progress.CurrentOperation.ToString()}");
            }
        }
        private void Verbose_DataAdded(object? sender, DataAddedEventArgs e)
        {
            if (sender != null)
            {
                var verbose = GetNewRecord<VerboseRecord>(sender, e);
                Debug.WriteLine(System.Diagnostics.Debugger.IsAttached ? "" : $"CurrentOperation: {verbose.Message.ToString()}");
            }
        }
    }
}