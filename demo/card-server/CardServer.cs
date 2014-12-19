namespace wyc
{
	using System;
    using System.Reflection;

    using Lite;

    using log4net;

    using Photon.SocketServer;

    public class CardServer : LiteApplication
    {
        #region Constants and Fields

        /// <summary>
        ///   An <see cref = "ILog" /> instance used to log messages to the log4net framework.
        /// </summary>
        private static readonly ILog log = LogManager.GetLogger(MethodBase.GetCurrentMethod().DeclaringType);

        #endregion

        #region Methods

		public CardServer()
		{
			this.Center = new UserCenter();
		}

        protected override PeerBase CreatePeer(InitRequest initRequest)
        {
			return new CardPeer(initRequest.Protocol, initRequest.PhotonPeer);
        }

        #endregion

		public UserCenter Center { get; set; }
    }
}