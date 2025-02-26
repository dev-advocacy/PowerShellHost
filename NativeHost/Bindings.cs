using HostConsoleManage;
using System;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Management.Automation;
using System.Management.Automation.Runspaces;
using System.Reflection;
using System.Runtime.InteropServices;

namespace NativeHost
{
    public static class Bindings
    {
        public static string AssemblyDirectory
        {
            get
            {
                // Get Executable directory
                // NativeHost\bin\Release\net8.0\publish
                if (Environment.ProcessPath != null)
                {
                    var executablepath = Path.GetDirectoryName(Environment.ProcessPath);
                    if (executablepath == null)
                    {
                        return string.Empty;
                    }
                    string assemblypath = Path.Combine(executablepath, "publish");
                    if (assemblypath != null)
                    {

                        return assemblypath;
                    }
                }
                return string.Empty;

            }
        }

        static Bindings()
        {
            AppDomain.CurrentDomain.AssemblyResolve += OnAssemblyResolve;
        }

        private static Assembly? OnAssemblyResolve(object? sender, ResolveEventArgs args)
        {
            // Implement the logic to locate and load the required assembly
            // For example, you can load the assembly from a specific path
            // get the assembly path 
            if (sender == null) {
                return null;
            }
            // get the assembly path           
            string assemblyName = new AssemblyName(args.Name).Name + ".dll";
            string fullPath = System.IO.Path.Combine(AssemblyDirectory, assemblyName);

            if (System.IO.File.Exists(fullPath))
            {
                return Assembly.LoadFrom(fullPath);
            }

            return null;
        }

        [UnmanagedCallersOnly]
        public static IntPtr RunCommandLLM(IntPtr ptrCommand, IntPtr prtConnection)
        {
            if (ptrCommand == IntPtr.Zero)
            {
                return IntPtr.Zero;                
            }
            string? command = Marshal.PtrToStringUni(ptrCommand);

            if (prtConnection == IntPtr.Zero)
            {
                return IntPtr.Zero;
            }
            string? connection = Marshal.PtrToStringUni(prtConnection);



            if (command != null && connection != null)
            {
                ChatLLM chatLLM = new ChatLLM();
                var output = chatLLM.RunCommandLLM(command, connection);
                return Marshal.StringToHGlobalUni(output);
            }
            return IntPtr.Zero;
        }
        [UnmanagedCallersOnly]
        public static IntPtr PowerShell_Create(int policy)
        {
            RunPowershellScript? ps = new RunPowershellScript();
            IntPtr ptrHandle = IntPtr.Zero;
            if (policy > 0 && policy < 6)
            {
                ps.InitializePowerShell((Microsoft.PowerShell.ExecutionPolicy)policy);
            }
            else
            {
                ps.InitializePowerShell(Microsoft.PowerShell.ExecutionPolicy.Unrestricted);
            }
            GCHandle gch = GCHandle.Alloc(ps, GCHandleType.Normal);
            ptrHandle = GCHandle.ToIntPtr(gch);
            return ptrHandle;
        }
        [UnmanagedCallersOnly]
        public static void PowerShell_AddCommand(IntPtr ptrHandle, IntPtr ptrPowerShellFile)
        {
            GCHandle gch = GCHandle.FromIntPtr(ptrHandle);
            if (!gch.IsAllocated)
            {
                return;
            }

            if (gch.Target != null)
            {
                var ps = (RunPowershellScript) gch.Target;
                
                string? command = Marshal.PtrToStringUni(ptrPowerShellFile);
                ps.ExecutePowerShellFile(command, null);
            }
        }

        [UnmanagedCallersOnly]
        public static IntPtr PowerShell_Execute_File(IntPtr ptrHandle, IntPtr ptrPowerShellFile)
        {
            GCHandle gch = GCHandle.FromIntPtr(ptrHandle);
            if (!gch.IsAllocated)
            {
                return IntPtr.Zero;
            }
            if (gch.Target != null)
            {
                RunPowershellScript ps = (RunPowershellScript) gch.Target;
                string? PowerShellFile = Marshal.PtrToStringUni(ptrPowerShellFile);
                if (PowerShellFile != null)
                {
                    var output = ps.ExecutePowerShellFile(PowerShellFile, null);
                    return Marshal.StringToHGlobalUni(output);
                }
            }
            return IntPtr.Zero;
        }

        [UnmanagedCallersOnly]
        public static IntPtr PowerShell_Execute_Script(IntPtr ptrHandle, IntPtr ptrPowerShellScript)
        {
            GCHandle gch = GCHandle.FromIntPtr(ptrHandle);
            if (!gch.IsAllocated)
            {
                return IntPtr.Zero;
            }
            if (gch.Target != null)
            {
                RunPowershellScript ps = (RunPowershellScript)gch.Target;
                string? PowerShellScript = Marshal.PtrToStringUni(ptrPowerShellScript);
                if (PowerShellScript != null)
                {
                    var output = ps.ExecutePowerShellScript(PowerShellScript);
                    return Marshal.StringToHGlobalUni(output);
                }
            }
            return IntPtr.Zero;
        }
    }
}
