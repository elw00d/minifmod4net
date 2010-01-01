using System;
using System.IO;
using System.Threading;
using minifmod4net;

namespace minifmod4net_test
{
    class Program
    {
        static void Main(string[] args)
        {
            MiniFmodModuleRegistry registry = new MiniFmodModuleRegistry();
            
            byte[] bytes = File.ReadAllBytes("test.xm");
            using (MiniFmodModule module = registry.LoadFromMemory(1, bytes)) {
                module.Play();

                do {
                    Console.Clear();

                    Console.WriteLine("Order = {0}", module.GetCurrentOrder());
                    Console.WriteLine("Row = {0}", module.GetCurrentRow());
                    Console.WriteLine("Time = {0}", module.GetCurrentTime());


                    if (Console.KeyAvailable) {
                        ConsoleKeyInfo info = Console.ReadKey();
                        if (info.Key == ConsoleKey.Escape) {
                            break;
                        }
                    }

                    Thread.Sleep(200);
                } while (true);

                module.Stop();
            }
        }
    }
}
