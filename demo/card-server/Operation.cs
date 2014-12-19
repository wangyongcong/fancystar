namespace wyc
{
	using System;
	using System.Collections;
	using System.Collections.Generic;
	using Photon.SocketServer;
	using Photon.SocketServer.Rpc;
	using ExitGames.Logging;

	public class Operations
	{
		public static bool Validate(OperationRequest request)
		{
			byte code = request.OperationCode;
			Dictionary<byte, Type> param;
			if (!Protocols.TryGetValue(code, out param))
			{
			//	log.DebugFormat("No such operation");
				return false;
			}
			var ht = request.Parameters;
			object obj;
			foreach (KeyValuePair<byte, Type> item in param)
			{
				if (!ht.TryGetValue(item.Key, out obj))
				{
				//	log.DebugFormat("No such param code: {0}", item.Key);
					return false;
				}
				if (obj.GetType() != item.Value)
				{
				//	log.DebugFormat("Type not match");
					return false;
				}
			}
			return true;
		}

		public enum Codes : byte
		{
			OP_ECHO = 1,
			OP_LOGIN,
			OP_LOGOUT,
			OP_UPLOAD_PACKET,
			OP_DOWNLOAD_PACKET,
		}

		public static readonly Dictionary<byte, Dictionary<byte, Type>> Protocols = new Dictionary<byte, Dictionary<byte, Type>>()
		{
			{ (byte)Codes.OP_ECHO, new Dictionary<byte,Type>() {
				{0, typeof(string)}, // words
			}},
			{ (byte)Codes.OP_LOGIN, new Dictionary<byte,Type>() {
				{0, typeof(string)}, // user name
				{1, typeof(string)}, // password	
			}},
			{ (byte)Codes.OP_LOGOUT, new Dictionary<byte,Type>() {
			}},
			{ (byte)Codes.OP_UPLOAD_PACKET, new Dictionary<byte,Type>() {
				{1, typeof(byte[])}, // content
			}},
			{ (byte)Codes.OP_DOWNLOAD_PACKET, new Dictionary<byte,Type>() {
				{1, typeof(uint)}, // revision, 0 means the latest version
			}},
		};

		public enum EventCodes : byte
		{
			EV_LOGIN_RESULT = 1,
		}
	}

}
