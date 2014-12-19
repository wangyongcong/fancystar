#ifndef __HEADER_WYC_GWEN_PWD
#define __HEADER_WYC_GWEN_PWD

#include "gwen/Controls/TextBox.h"

namespace Gwen
{
	namespace Controls
	{
		class SecureText : public TextBox
		{
			GWEN_CONTROL( SecureText, TextBox );
			virtual void InsertText( const Gwen::UnicodeString& str );
			virtual void DeleteText( int iStartPos, int iLength );
			const Gwen::UnicodeString& GetSecureText() const {
				return m_secure_text;
			}
		protected:
			Gwen::UnicodeString m_secure_text;
		};
	} // namespace Controls

} // namespace Gwen

#endif //__HEADER_WYC_GWEN_PWD

