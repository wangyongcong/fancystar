namespace wyc
{
	using System;
	using System.Collections;
	using System.Collections.Generic;

	using ExitGames.Concurrency.Fibers;
	using ExitGames.Logging;

	public class DBLogger : IDBLog
	{
		private ILogger _log = null;

		public DBLogger(ILogger log)
		{
			this._log = log;
		}

		void IDBLog.Error(string format, params object[] arg)
		{
			if (this._log != null)
				this._log.ErrorFormat(format, arg);
		}
	}

	public class UserCenter : IDisposable
	{

		#region Constants and Fields

		protected static readonly ILogger log = LogManager.GetCurrentClassLogger();

		private DataCenter _db = null;

		private Dictionary<string, GameUser> _users = null;

		#endregion

		#region Constructors and Destructors

		public UserCenter()
		{
			this.ExecutionFiber = new PoolFiber();
			this.ExecutionFiber.Start();

			DBLogger logger = new DBLogger(UserCenter.log);
			this._db = new DataCenter(logger);
			this._users = new Dictionary<string, GameUser>();

			this.Packet = this._db.LoadPacket(0);
		}

		~UserCenter()
		{
			this.Dispose(false);
		}

		public void Dispose()
		{
			this.Dispose(true);
			GC.SuppressFinalize(this);
		}

		protected virtual void Dispose(bool dispose)
		{
			this.IsDisposed = true;

			if (dispose)
			{
				this.ExecutionFiber.Dispose();
			}
		}

		#endregion

		#region Attributes

		public bool IsDisposed { get; private set; }

		public PoolFiber ExecutionFiber { get; private set; }

		public FileRecord Packet { get; private set; }

		#endregion

		#region Methods

		public void SendMessage(Message msg)
		{
			this.ExecutionFiber.Enqueue(() => this.ProcessMessage(msg));
		}

		public void ProcessMessage(Message msg)
		{
			MessageHandler handler;
			if (_message_map.TryGetValue(msg.Id, out handler))
			{
				handler(this, msg);
			}
			else
			{
				log.ErrorFormat("Unknown message: {0}",msg.Id);
			}
		}

		#endregion

		#region Message Handlers

		delegate void MessageHandler(UserCenter self, Message msg);

		public enum MessageCodes : byte
		{
			MSG_LOGIN = 1,
			MSG_LOGOUT,
			MSG_SAVE_PACKET,
			MSG_LOAD_PACKET,
		}
		private static Dictionary<byte, MessageHandler> _message_map = new Dictionary<byte, MessageHandler>()
		{
			{ (byte)(MessageCodes.MSG_LOGIN), UserCenter.MsgLogin },
			{ (byte)(MessageCodes.MSG_LOGOUT), UserCenter.MsgLogout },
			{ (byte)(MessageCodes.MSG_SAVE_PACKET), UserCenter.MsgSavePacket },
			{ (byte)(MessageCodes.MSG_LOAD_PACKET), UserCenter.MsgLoadPacket },
		};

		static void MsgLogin(UserCenter self, Message msg)
		{
			if (!(msg.Param is ArrayList))
				return;
			ArrayList al = (ArrayList)msg.Param;
			if (al.Count < 3)
				return;
			CardPeer peer = (CardPeer)al[0];
			string usr = (string)al[1];
			string pwd = (string)al[2];

			GameUser user = null;
			if (self._users.ContainsKey(usr))
			{
				user = self._users[usr];
			}
			else
			{
				user = self._db.ValidateUser(usr, pwd);
				if (user != null)
				{
					self._users.Add(usr, user);
				}
				else if(log.IsDebugEnabled) // user not found
				{
					log.Debug(string.Format("User not found: name={0}, pwd={1}", usr, pwd));
				}
			}
			Message ret = new Message((byte)CardPeer.MessageCodes.MSG_LOGIN_RESULT);
			ret.Param = user;
			peer.SendMessage(ret);
		}

		static void MsgLogout(UserCenter self, Message msg)
		{
			if (!(msg.Param is string))
				return;
			self._users.Remove((string)msg.Param);
		}

		static void MsgSavePacket(UserCenter self, Message msg)
		{
			if (!(msg.Param is string))
				return;
			self._db.SavePacket(msg.Param as byte[]);
		}

		static void MsgLoadPacket(UserCenter self, Message msg)
		{
			ArrayList al = msg.Param as ArrayList;
			if (al == null)
				return;
			CardPeer peer = al[0] as CardPeer;
			uint revision = (uint)al[1];
			FileRecord file = self._db.LoadPacket(revision);
			Message ret = new Message((byte)CardPeer.MessageCodes.MSG_LOAD_PACKET_RESULT);
			ret.Param = file;
			peer.SendMessage(ret);
		}

		#endregion // Message Handlers
	}

}
