namespace wyc
{

	public class Message
	{

		public Message(byte id)
		{
			this.Id = id;
			this.Param = null;
		}

		public byte Id { get; protected set; }

		public object Param { get; set; }
	}

} // namespace wyc
