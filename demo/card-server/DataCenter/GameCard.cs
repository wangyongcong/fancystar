namespace wyc
{
	using System;

	using MongoDB.Bson;
	using MongoDB.Bson.Serialization;
	using MongoDB.Bson.Serialization.Attributes;
	using MongoDB.Bson.Serialization.IdGenerators;

	enum GameCardState
	{
		CARD_IN_USE = 1,
	};

	public class GameCard
	{
		public ObjectId Id = ObjectId.Empty;

		[BsonElement("type")]
		public uint Type = 0;

		[BsonElement("owner")]
		public ObjectId Owner = ObjectId.Empty;

		[BsonElement("state")]
		public uint State = 0;

		[BsonIgnore]
		public bool InUse
		{
			get
			{
				return (this.State & (uint)(GameCardState.CARD_IN_USE))!=0;
			}
			set
			{
				if(value)
					this.State |= (uint)(GameCardState.CARD_IN_USE);
				else
					this.State &= ~(uint)(GameCardState.CARD_IN_USE);
			}
		}
	};
}
