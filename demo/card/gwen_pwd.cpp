#include "fscorepch.h"
#include "gwen_pwd.h"

namespace Gwen
{
	namespace Controls
	{
		GWEN_CONTROL_CONSTRUCTOR(SecureText)
		{
		}

		void SecureText::InsertText( const Gwen::UnicodeString& strInsert )
		{
			if ( HasSelection() )
			{
				EraseSelection();
			}

			if ( m_iCursorPos > TextLength() ) m_iCursorPos = TextLength();

			if ( !IsTextAllowed( strInsert, m_iCursorPos )  )
				return;

			m_secure_text.insert(m_iCursorPos, strInsert);

			UnicodeString str;
			str.assign(m_secure_text.length(),L'*');
			SetText( str );

			m_iCursorPos += (int) strInsert.size();
			m_iCursorEnd = m_iCursorPos;

			RefreshCursorBounds();
		}

		void SecureText::DeleteText( int iStartPos, int iLength )
		{
			m_secure_text.erase(iStartPos, iLength);
			UnicodeString str;
			str.assign(m_secure_text.length(),L'*');
			SetText( str );

			if ( m_iCursorPos > iStartPos )
			{
				SetCursorPos( m_iCursorPos - iLength );
			}

			SetCursorEnd( m_iCursorPos );
		}

	} // namespace Controls

} // namespace Gwen
