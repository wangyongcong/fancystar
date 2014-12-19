namespace wyc
{
	using System;

	using MongoDB.Driver;
	using MongoDB.Driver.Builders;
	using MongoDB.Bson;
	using MongoDB.Bson.Serialization;
	using MongoDB.Bson.Serialization.Attributes;

	public interface IDBLog
	{
		void Error(string format, params object[] arg);
	};

	public class ConsoleLogger : IDBLog
	{
		void IDBLog.Error(string format, params object[] arg)
		{
			Console.WriteLine(format, arg);
		}
	}

	public class DataCenter
	{
		private MongoDatabase _database = null;
		private MongoCollection<GameUser> _dbUsers = null;
		private MongoCollection<GameCard> _dbCards = null;
		private MongoCollection<FileRecord> _dbFiles = null;

		private IDBLog _log = null;

		public DataCenter(IDBLog log = null)
		{
			this._log = log;
			if (this._log == null)
				this._log = new ConsoleLogger();

			// register class mappings
			BsonClassMap.RegisterClassMap<GameUser>();

			// link to mongodb
			// mongodb://[username:password@]hostname[:port][/[database][?options]]
			MongoServer svr = MongoServer.Create("mongodb://ycwang:netease3984@localhost:27017/?safe=true");
			this._database = svr.GetDatabase("CardGame");
			this._dbUsers = this._database.GetCollection<GameUser>("users");
			this._dbUsers.EnsureIndex(IndexKeys.Ascending("email"), IndexOptions.SetUnique(true));
			this._dbCards = this._database.GetCollection<GameCard>("cards");
			this._dbCards.EnsureIndex(IndexKeys.Ascending("type"));
			this._dbCards.EnsureIndex(IndexKeys.Ascending("owner"));
			this._dbFiles = this._database.GetCollection<FileRecord>("packets");
			this._dbFiles.EnsureIndex(IndexKeys.Ascending("name").Descending("revision"), IndexOptions.SetUnique(true));
		}

		~DataCenter()
		{
		}

		public bool CreateUser(string email, string pwd, string nick, out int error)
		{
			error = 0;
			var cursor = this._dbUsers.Find(Query.EQ("email", email));
			if (cursor.Count() != 0)
			{
				// email address already exists
				error = 1;
				return false;
			}
			string[] tmp = email.Split('@');
			if (tmp.Length != 2 || tmp[0].Length == 0 || tmp[1].Length == 0)
			{
				// invalid email address
				error = 2;
				return false;
			}
			GameUser new_user = new GameUser();
			new_user.Email = email;
			if (pwd.Length == 0)
				new_user.Password = "guest";
			else
				new_user.Password = pwd;
			if (nick.Length == 0)
				new_user.NickName = "Guest";
			else
				new_user.NickName = nick;
			new_user.Auth = 0;
			new_user.RegDate = DateTime.Now;
			new_user.GameCount = 0;
			new_user.WinCount = 0;

			try
			{
				this._dbUsers.Insert(new_user);
			}
			catch (Exception ex)
			{
    			this._log.Error(ex.ToString());
				error = 3;
				return false;
			}
			return true;
		}

		public GameUser ValidateUser(string email, string pwd)
		{
			GameUser user = null;
			var cursor = this._dbUsers.Find(Query.EQ("email", email));
			var iterator = cursor.GetEnumerator();
			if (iterator.MoveNext() && pwd == iterator.Current.Password)
			{
				user = iterator.Current;
			}
			iterator.Dispose();
			if (user!=null)
			{
				var mycards = this._dbCards.Find(Query.EQ("owner", user.Id));
				foreach (GameCard card in mycards)
				{
					user.PushCard(card);
				}
			}
			return user;
		}

		public bool ChangePassword(string email, string old_pwd, string new_pwd)
		{
			var cursor = this._dbUsers.Find(Query.EQ("email", email));
			var iterator = cursor.GetEnumerator();
			bool ok = false;
			if (iterator.MoveNext())
			{
				GameUser user = iterator.Current;
				if (old_pwd == user.Password)
				{
					user.Password = new_pwd;
					try
					{
						this._dbUsers.Save(user);
						ok = true;
					}
					catch (Exception ex)
					{
						Console.WriteLine(ex.ToString());
					}
				}
			}
			iterator.Dispose();
			return ok;
		}

		public GameCard AcquireGameCard(GameUser user, uint type)
		{
			var query = Query.And(Query.EQ("type",type),Query.EQ("owner",ObjectId.Empty));
			GameCard card = this._dbCards.FindOne(query);
			if (card == null)
			{
				card = new GameCard();
				card.Type = type;
			}
			card.Owner = user.Id;
			try
			{
				this._dbCards.Save(card);
			}
			catch(Exception ex)
			{
			    this._log.Error(ex.ToString());
				return null;
			}
			user.PushCard(card);
			return card;
		}

		public bool DiscardGameCard(GameUser user, ObjectId cid)
		{
			GameCard card = user.PopCard(cid);
			if (card == null)
				return false;
			card.InUse = false;
			card.Owner = ObjectId.Empty;
			try
			{
				this._dbCards.Save(card);
			}
			catch (Exception ex)
			{
				this._log.Error(ex.ToString());
				return false;
			}
			return true;
		}

		public GameCard FindCard(ObjectId cid)
		{
			var query = Query.EQ("_id", cid);
			GameCard card = this._dbCards.FindOne(query);
			return card;
		}

		public bool SavePacket(byte [] data)
		{
			string fileName = "card_packet.json";
			var query = Query.EQ("name", fileName);
			var cursor = this._dbFiles.Find(query);
			uint latest = (uint)cursor.Count();
			FileRecord file = new FileRecord();
			file.Name = fileName;
			file.Revision = latest+1;
			file.Date = DateTime.Now;
			file.Content = data;
			try
			{
				this._dbFiles.Insert(file);
			}
			catch (Exception ex)
			{
				this._log.Error(ex.ToString());
				return false;
			}
			return true;
		}

		public FileRecord LoadPacket(uint revision)
		{
			string fileName = "card_packet.json";
			FileRecord file = null;
			if (revision != 0) // find the exact revision
			{
				var query = Query.And(Query.EQ("name", fileName), Query.EQ("revision", revision));
				file = this._dbFiles.FindOne(query);
			}
			else // find the latest revision
			{
				var query = Query.EQ("name", fileName);
				var cursor = this._dbFiles.Find(query);
				cursor.SetSortOrder(SortBy.Descending("revision")).SetLimit(1);
				foreach (FileRecord rec in cursor)
				{
					file = rec;
					break;
				}
			}
			return file;
		}
	};
}
