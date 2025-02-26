using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;


namespace HostLibrary
{
    public class RevolveAssembly
    {
        public static string PlatformSpecificDir
        {
            get { return (IntPtr.Size == 8) ? "x64" : "x86"; }
        }
        public static string OSSpecificDir()
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
            {
                return "unix";
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                return "osx";
            }
            else
            {
                return "win";
            }
        }
 
        Assembly? currentDomain_AssemblyResolve(object sender, ResolveEventArgs args)
        {
            var splitIndex = args.Name.IndexOf(',');
            var assemblyFileName = String.Empty;
            if (splitIndex > 0)
                assemblyFileName = args.Name.Substring(0, splitIndex) + ".dll";
            else
                assemblyFileName = args.Name + ".dll";
            var executablepath = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            if (executablepath == null)
            {
                Console.WriteLine("Could not load assembly: {0}", assemblyFileName);
                return null;
            }
            // first look in general (x86 and x64) lib path
            var assemblyPath = Path.Combine(executablepath,  @"net8.0" , assemblyFileName); // path must be absolute to load assembly
            if (File.Exists(assemblyPath))
            {
                var asm = Assembly.LoadFile(assemblyPath);
                return asm;
            }
            // not found there, so look in platform specific path
            assemblyPath = Path.Combine(executablepath, @"net8.0\runtimes", OSSpecificDir(), @"lib\net8.0", assemblyFileName);
            if (File.Exists(assemblyPath))
            {
                var asm = Assembly.LoadFile(assemblyPath);
                return asm;
            }

            assemblyPath = Path.Combine(executablepath, @"net8.0\runtimes", OSSpecificDir() + "-" + PlatformSpecificDir, "lib\\netstandard1.6", assemblyFileName);
            if (File.Exists(assemblyPath))
            {
                var asm = Assembly.LoadFile(assemblyPath);
                return asm;
            }
            Console.WriteLine("LibPathManager - Could not load assembly: {0}", assemblyFileName);
            return null;
            
        }
        public RevolveAssembly()
        {
            AppDomain currentDomain = AppDomain.CurrentDomain;
            currentDomain.AssemblyResolve += new ResolveEventHandler(currentDomain_AssemblyResolve);
        }
        public Assembly Load(string assemblyString)
        {
            return Assembly.Load(assemblyString);
        }
    }

    public static class Lib
    {
        private static int s_CallCount = 1;

        [StructLayout(LayoutKind.Sequential)]
        public struct LibArgs
        {
            public IntPtr Message;
            public int Number;
        }

        

        public static int Hello(IntPtr arg, int argLength)
        {
            if (argLength < System.Runtime.InteropServices.Marshal.SizeOf(typeof(LibArgs)))
            {
                return 10;
            }


            RevolveAssembly revolveAssembly = new RevolveAssembly();
            //revolveAssembly.Load("System.Management.Automation");


            LibArgs libArgs = Marshal.PtrToStructure<LibArgs>(arg);


            Console.WriteLine("init");

            if (libArgs.Message == IntPtr.Zero)
            {
                Console.WriteLine("-- message: null");
                Console.WriteLine($"-- number: {libArgs.Number}");
                return -1;
            }

            string? message = RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
                ? Marshal.PtrToStringUni(libArgs.Message)
                : Marshal.PtrToStringUTF8(libArgs.Message);

            Console.WriteLine($"-- message: {message}");
            Console.WriteLine($"-- number: {libArgs.Number}");

            if (message != null)
            {
                HostLibrary.HostedRunspace hostedRunspace = new HostedRunspace();

                // create an empty array of string
                string[] modulesToLoad = new string[0];

                // read the file from the disk using message as the file path
                string scriptContents = System.IO.File.ReadAllText(message);

                try
                {
                    if (scriptContents != null)
                    {
                        // run the script
                        hostedRunspace.InitializeRunspaces(0, 10, modulesToLoad);
                        hostedRunspace.RunScript(scriptContents, null);
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"exception {ex}");
                }
            }

            return 0;
        }

        public delegate void CustomEntryPointDelegate(LibArgs libArgs);
        public static void CustomEntryPoint(LibArgs libArgs)
        {
            Console.WriteLine($"Hello, world! from {nameof(CustomEntryPoint)} in {nameof(Lib)}");
            PrintLibArgs(libArgs);
        }

        [UnmanagedCallersOnly]
        public static void CustomEntryPointUnmanagedCallersOnly(LibArgs libArgs)
        {
            Console.WriteLine($"Hello, world! from {nameof(CustomEntryPointUnmanagedCallersOnly)} in {nameof(Lib)}");
            PrintLibArgs(libArgs);
        }

        private static void PrintLibArgs(LibArgs libArgs)
        {
            string message = RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
                ? Marshal.PtrToStringUni(libArgs.Message)
                : Marshal.PtrToStringUTF8(libArgs.Message);

            Console.WriteLine($"-- message: {message}");
            Console.WriteLine($"-- number: {libArgs.Number}");
        }
    }
}
