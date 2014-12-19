namespace wyc
{
	using System.Collections;
	using System.Collections.Generic;
	using System.Security.Cryptography;
	using System.Text;

    using ExitGames.Logging;
    using Photon.SocketServer;
    using PhotonHostRuntimeInterfaces;
    using Lite;
    using Lite.Caching;
    using Lite.Operations;

	public enum UserAuth : uint
	{
		NORMAL = 0,
		DESIGNER = 1,
	}

    public class CardPeer : LitePeer
    {
        #region Constants and Fields

        /// <summary>
        ///   An <see cref = "ILogger" /> instance used to log messages to the logging framework.
        /// </summary>
        private static readonly ILogger log = LogManager.GetCurrentClassLogger();

		private GameUser _gameUser = null;

		#endregion //  Constants and Fields

		#region Constructors and Destructors

		public CardPeer(IRpcProtocol rpcProtocol, IPhotonPeer peer)
            : base(rpcProtocol, peer)
        {
        }

		#endregion // Constructors and Destructors

		#region Methods

		protected override RoomReference GetRoomReference(JoinRequest joinRequest)
        {
            return CardGameCache.Instance.GetRoomReference(joinRequest.GameId, this);
        }

        protected override void OnOperationRequest(OperationRequest operationRequest, SendParameters sendParameters)
        {
            if (log.IsDebugEnabled)
            {
                log.DebugFormat("OnOperationRequest. Code={0}", operationRequest.OperationCode);
            }

			OperationHandler handler;
			if (CardPeer._operation_map.TryGetValue(operationRequest.OperationCode, out handler))
			{
				handler(this, operationRequest, sendParameters);
				return;
			}

            base.OnOperationRequest(operationRequest, sendParameters);
            return;
        }

		public void SendMessage(Message msg)
		{
			this.RequestFiber.Enqueue(() => this.ProcessMessage(msg));
		}

		public void ProcessMessage(Message msg)
		{
			MessageHandler handler;
			if (_message_map.TryGetValue(msg.Id, out handler))
			{
				handler(this, msg);
				return;
			}
			if(log.IsDebugEnabled)
				log.DebugFormat("Unknown message: {0}",msg.Id);
		}

        #endregion // Methods

		#region Operation Handlers

		delegate void OperationHandler(CardPeer self, OperationRequest req, SendParameters param);

		private static Dictionary<byte, OperationHandler> _operation_map = new Dictionary<byte, OperationHandler>()
		{
			{ (byte)(Operations.Codes.OP_ECHO), CardPeer.OperationEcho },
			{ (byte)(Operations.Codes.OP_LOGIN), CardPeer.OperationLogin },
			{ (byte)(Operations.Codes.OP_LOGOUT), CardPeer.OperationLogout },
			{ (byte)(Operations.Codes.OP_UPLOAD_PACKET), CardPeer.OperationUploadPacket },
			{ (byte)(Operations.Codes.OP_DOWNLOAD_PACKET), CardPeer.OperationDownloadPacket },
		};

		static void OperationEcho(CardPeer self, OperationRequest request, SendParameters sendParameters)
		{
			if (!Operations.Validate(request))
			{
				return;
			}
			string words = (string)request[0];
			var response = new OperationResponse(request.OperationCode);
			response.Parameters = new Dictionary<byte, object>()
			{
				{0,words},
			};
			self.SendOperationResponse(response, sendParameters);
		}

		static void OperationLogin(CardPeer self, OperationRequest request, SendParameters sendParameters)
		{
			if (!Operations.Validate(request))
			{
				return;
			}
			string usr, pwd;
			usr = (string)request[0];
			pwd = (string)request[1];
			pwd += "FANCY;STAR";
			MD5 md5hash = MD5.Create();
			byte [] data = md5hash.ComputeHash(Encoding.UTF8.GetBytes(pwd));
			StringBuilder builder = new StringBuilder();
			for(int i=0; i<data.Length; ++i)
			{
				builder.Append(data[i].ToString("x2"));
			}
			pwd = builder.ToString();
			
			ArrayList al = new ArrayList();
			al.Add(self);
			al.Add(usr);
			al.Add(pwd);
			Message msg = new Message((byte)UserCenter.MessageCodes.MSG_LOGIN);
			msg.Param = al;
			CardServer app = (CardServer)(ApplicationBase.Instance);
			app.Center.SendMessage(msg);
		}

		static void OperationLogout(CardPeer self, OperationRequest request, SendParameters sendParameters)
		{
			if (!Operations.Validate(request))
				return;
			if (self._gameUser == null)
				return;
			CardServer app = (CardServer)(ApplicationBase.Instance);
			Message msg = new Message((byte)UserCenter.MessageCodes.MSG_LOGOUT);
			msg.Param = self._gameUser.Email;
			app.Center.SendMessage(msg);
			self._gameUser = null;
		}

		static void OperationUploadPacket(CardPeer self, OperationRequest request, SendParameters send)
		{
			if (!Operations.Validate(request))
				return;
			if (self._gameUser == null)
				return;
			OperationResponse response = new OperationResponse(request.OperationCode);
			byte ret = 0;
			if (0 == (self._gameUser.Auth & (uint)UserAuth.DESIGNER))
			{
				ret = 1;
			}
			else
			{
				CardServer app = (CardServer)(ApplicationBase.Instance);
				Message msg = new Message((byte)UserCenter.MessageCodes.MSG_SAVE_PACKET);
				msg.Param = request[0];
				app.Center.SendMessage(msg);
			}
			response.Parameters = new Dictionary<byte, object>()
			{
				{0,ret},
			};
			self.SendOperationResponse(response, send);
		}

		static void OperationDownloadPacket(CardPeer self, OperationRequest request, SendParameters send)
		{
			if (!Operations.Validate(request))
				return;
			if (self._gameUser == null)
				return;
			CardServer app = (CardServer)(ApplicationBase.Instance);
			Message msg = new Message((byte)UserCenter.MessageCodes.MSG_LOAD_PACKET);
			msg.Param = request[0];
			app.Center.SendMessage(msg);
		}

		#endregion // Operation Handlers


		#region Message Handlers

		delegate void MessageHandler(CardPeer self, Message msg);

		public enum MessageCodes : byte
		{
			MSG_LOGIN_RESULT,
			MSG_LOAD_PACKET_RESULT,
		}

		private static Dictionary<byte, MessageHandler> _message_map = new Dictionary<byte, MessageHandler>()
		{
			{ (byte)(MessageCodes.MSG_LOGIN_RESULT), CardPeer.MsgLoginResult },
			{ (byte)(MessageCodes.MSG_LOAD_PACKET_RESULT), CardPeer.MsgLoadPacketResult },
		};
		
		static void MsgLoginResult(CardPeer self, Message msg)
		{
			int result;
			if (msg.Param is GameUser)
			{
				self._gameUser = (GameUser)msg.Param;
				result = 0;
			}
			else
			{
				self._gameUser = null;
				result = 1;
			}
			OperationResponse response = new OperationResponse((byte)Operations.Codes.OP_LOGIN);
			response.Parameters = new Dictionary<byte, object>() { 
				{0, (object)result},
			};
			self.SendOperationResponse(response, new SendParameters());
		}

		static void MsgLoadPacketResult(CardPeer self, Message msg)
		{
			FileRecord file = msg.Param as FileRecord;
			OperationResponse response = new OperationResponse((byte)Operations.Codes.OP_DOWNLOAD_PACKET);
			if (file != null)
			{
				response.Parameters = new Dictionary<byte, object>()
				{
					{0, file.Revision},
					{1, file.Content},
				};
			}
			else
			{
				response.Parameters = new Dictionary<byte, object>()
				{
					{0, 0},
				};
			}
			self.SendOperationResponse(response, new SendParameters());
		}

		#endregion // Message Handlers
	}
}