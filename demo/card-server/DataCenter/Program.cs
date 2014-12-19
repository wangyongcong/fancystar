using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DataCenter
{
	using wyc;
	class Program
	{
		static void Main(string[] args)
		{
			DataCenter dc = new DataCenter();
			string path = "E:\\dev\\fancystar\\bin\\demo-card\\data\\card_packet.json";
			if (!File.Exists(path))
			{
				Console.WriteLine("File not found");
				return;
			}
			byte [] content = File.ReadAllBytes(path);
			Console.WriteLine("Saving content...");
			FileRecord packet;
			if (dc.SavePacket(content))
			{
				Console.WriteLine("Save OK");
				packet = dc.LoadPacket(0);
				Console.WriteLine("File: {0}\nRevision: {1}",packet.Name,packet.Revision);
				if (!packet.Content.SequenceEqual(content))
				{
					Console.WriteLine("Content not match");
				}
			}
			else
			{
				Console.WriteLine("Failed to save");
				return;
			}
			Console.ReadLine();
		}
	}
}
