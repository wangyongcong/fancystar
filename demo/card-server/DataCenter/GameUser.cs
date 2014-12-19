namespace wyc
{
	using System;
	using System.Collections;

	using MongoDB.Bson;
	using MongoDB.Bson.Serialization;
	using MongoDB.Bson.Serialization.Attributes;
	using MongoDB.Bson.Serialization.IdGenerators;

	public class GameUser
	{
		public ObjectId Id = ObjectId.Empty;

		[BsonIgnore]
		public ArrayList CardBag = new ArrayList();

		[BsonElement("email")]
		public string Email;

		[BsonElement("pwd")]
		public string Password;

		[BsonElement("nick")]
		public string NickName;

		[BsonElement("auth")]
		public int Auth;

		[BsonElement("regdate")]
		public DateTime RegDate;

		[BsonElement("game")]
		public UInt32 GameCount;

		[BsonElement("win")]
		public UInt32 WinCount;

		public void PushCard(GameCard card)
		{
			this.CardBag.Add(card);
		}

		public GameCard PopCard(ObjectId cid)
		{
			GameCard card;
			for (int i = 0, count = this.CardBag.Count; i < count; ++i)
			{
				card = (GameCard)(this.CardBag[i]);
				if (cid == card.Id)
				{
					this.CardBag[i] = this.CardBag[count - 1];
					this.CardBag.RemoveAt(count - 1);
					return card;
				}
			}
			return null;
		}

		public GameCard GetCard(ObjectId cid)
		{
			GameCard card;
			for (int i = 0, count = this.CardBag.Count; i < count; ++i)
			{
				card = (GameCard)(this.CardBag[i]);
				if (cid == card.Id)
					return card;
			}
			return null;
		}
	};
}
