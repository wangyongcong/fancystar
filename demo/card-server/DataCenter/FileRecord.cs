namespace wyc
{
	using System;

	using MongoDB.Bson;
	using MongoDB.Bson.Serialization;
	using MongoDB.Bson.Serialization.Attributes;
	using MongoDB.Bson.Serialization.IdGenerators;

	public class FileRecord
	{
		public ObjectId Id = ObjectId.Empty;

		[BsonElement("name")]
		public string Name { get; set; }

		[BsonElement("revision")]
		public uint Revision { get; set; }

		[BsonElement("date")]
		public DateTime Date { get; set; }

		[BsonElement("content")]
		public byte[] Content { get; set; }

	}

}
