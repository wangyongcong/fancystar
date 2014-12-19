# encoding: utf8

import tkinter
import wycpy

if __name__=='__main__':
	print("*"*79)
	print("*  String Hash")
	print("*  version: 1.0")
	print("*  author: Yongcong Wang")
	print("*  python: 3.0")
	print("*"*79)
	tk=tkinter.Tk("string hash")
	tk.withdraw()
	while True:
		s=input("enter string: ")
		if s=="quit":
			break;
		code=wycpy.strhash(s.encode('utf8'));
		print("hash code: 0x%X"%code)
		s='%s = 0x%X,'%(s.upper(),code)
		tk.clipboard_clear()
		tk.clipboard_append(s)
	tk.destroy()
	print("terminate")
