using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace quadtreetester
{
    class Program
    {
        static void Main(string[] args)
        {
            int start = 0;
            int count = 5000;
            int step = 10;

            Process p = new Process();
            p.StartInfo.FileName = "quadtree.exe";
            p.StartInfo.UseShellExecute = false;
            p.StartInfo.RedirectStandardOutput = true;

            int pointCount = 30000;
            int pointsPerQuad = 4;

            int left = 4000;
            int top = 4000;
            int right = 6000;
            int bottom = 6000;
            
            using (TextWriter tr = File.CreateText("output.txt"))
            {
                //tr.WriteLine("Point count\tPoints per quad\tSearch area left\tSearch area top\tSearch area right\tSearch area bottom");

                Console.WriteLine("Pisteiden määrä\tNelipuun hakuaika\tPeräkkäishaun aika");
                tr.WriteLine("Pisteiden määrä\tNelipuun hakuaika\tPeräkkäishaun aika");

                for (int i = start; i <= count; i += step)
                {
                    //left = i;
                    //top = i;
                    //right = 10000 - i;
                    //bottom = 10000 - i;

                    pointCount = i;

                    //pointsPerQuad = i;

                    p.StartInfo.Arguments = "0 0 " + pointCount + " " + pointsPerQuad + " " + left + " " + top + " " + right + " " + bottom;

                    p.Start();
                    p.WaitForExit();

                    const string subStr = "OUTPUT";

                    string output = p.StandardOutput.ReadToEnd();
                    output = output.Substring(output.IndexOf(subStr) + subStr.Length);

                    output = output.Replace("\r", "");
                    output = output.Replace("\n", "");

                    string[] delims = new string[1] { "\t" };

                    List<string> values = output.Split(delims, StringSplitOptions.RemoveEmptyEntries).ToList();

                    //Console.Write((10000 - i * 2) * (10000 - i * 2));
                    //tr.Write((10000 - i * 2) * (10000 - i * 2));

                    Console.Write(pointCount);
                    tr.Write(pointCount);
                    Console.Write("\t");
                    tr.Write("\t");

                    Console.Write(values[0]);
                    tr.Write(values[0]);
                    Console.Write("\t");
                    tr.Write("\t");

                    Console.Write(values[1]);
                    tr.Write(values[1]);
                    Console.Write("\r\n");
                    tr.Write("\r\n");

                    tr.Flush();
                }
            }

            //Console.ReadKey();
        }
    }
}
