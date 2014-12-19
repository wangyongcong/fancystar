namespace wyc
{
    using Lite;
    using Lite.Caching;

    public class CardGameCache : RoomCacheBase
    {
        /// <summary>
        /// The singleton instance.
        /// </summary>
        public static readonly CardGameCache Instance = new CardGameCache();

        protected override Room CreateRoom(string roomId, params object[] args)
        {
            return new CardGame(roomId);
        }
    }
}
