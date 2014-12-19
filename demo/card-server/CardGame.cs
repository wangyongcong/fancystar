namespace wyc
{
    using Lite;

    using Photon.SocketServer;

    public class CardGame : LiteGame
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="CardGame"/> class.
        /// </summary>
        /// <param name="gameName">The name of the game.</param>
        public CardGame(string gameName)
            : base(gameName)
        {
        }

        /// <summary>
        /// Called for each operation in the execution queue.
        /// </summary>
        /// <param name="peer">The peer.</param>
        /// <param name="operationRequest">The operation request to execute.</param>
        /// <param name="sendParameters"></param>
        /// <remarks>
        /// ExecuteOperation is overriden to handle our custom operations.
        /// </remarks>
        protected override void ExecuteOperation(LitePeer peer, OperationRequest operationRequest, SendParameters sendParameters)
        {
             base.ExecuteOperation(peer, operationRequest, sendParameters);
        }

    }
}
