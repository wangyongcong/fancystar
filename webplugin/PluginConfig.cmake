#/**********************************************************\ 
#
# Auto-Generated Plugin Configuration file
# for Fancystar
#
#\**********************************************************/

set(PLUGIN_NAME "Fancystar")
set(PLUGIN_PREFIX "FAN")
set(COMPANY_NAME "Fancystar")

# ActiveX constants:
set(FBTYPELIB_NAME FancystarLib)
set(FBTYPELIB_DESC "Fancystar 1.0 Type Library")
set(IFBControl_DESC "Fancystar Control Interface")
set(FBControl_DESC "Fancystar Control Class")
set(IFBComJavascriptObject_DESC "Fancystar IComJavascriptObject Interface")
set(FBComJavascriptObject_DESC "Fancystar ComJavascriptObject Class")
set(IFBComEventSource_DESC "Fancystar IFBComEventSource Interface")
set(AXVERSION_NUM "1")

# NOTE: THESE GUIDS *MUST* BE UNIQUE TO YOUR PLUGIN/ACTIVEX CONTROL!  YES, ALL OF THEM!
set(FBTYPELIB_GUID 69eef737-3009-5828-be39-bec7189e3d91)
set(IFBControl_GUID c3e1014c-3226-5e3b-9a00-ff11149ae918)
set(FBControl_GUID fe0cc844-711a-5996-87e9-88b00c5e0c77)
set(IFBComJavascriptObject_GUID d797be41-120a-5054-ad0e-0c4f76d001a2)
set(FBComJavascriptObject_GUID e4a8d5a5-fb08-5780-9760-134ba9d11b79)
set(IFBComEventSource_GUID 68ec2264-def9-54c3-bbde-5dfe404067af)

# these are the pieces that are relevant to using it from Javascript
set(ACTIVEX_PROGID "Fancystar.Fancystar")
set(MOZILLA_PLUGINID "fancystar.org/Fancystar")

# strings
set(FBSTRING_CompanyName "Fancystar")
set(FBSTRING_FileDescription "Fancystar game player")
set(FBSTRING_PLUGIN_VERSION "1.0.0.0")
set(FBSTRING_LegalCopyright "Copyright 2011 Fancystar")
set(FBSTRING_PluginFileName "np${PLUGIN_NAME}.dll")
set(FBSTRING_ProductName "Fancystar")
set(FBSTRING_FileExtents "")
set(FBSTRING_PluginName "Fancystar")
set(FBSTRING_MIMEType "application/x-fancystar")

# Uncomment this next line if you're not planning on your plugin doing
# any drawing:

#set (FB_GUI_DISABLED 1)

# Mac plugin settings. If your plugin does not draw, set these all to 0
set(FBMAC_USE_QUICKDRAW 0)
set(FBMAC_USE_CARBON 1)
set(FBMAC_USE_COCOA 1)
set(FBMAC_USE_COREGRAPHICS 1)
set(FBMAC_USE_COREANIMATION 0)
set(FBMAC_USE_INVALIDATINGCOREANIMATION 0)

# If you want to register per-machine on Windows, uncomment this line
#set (FB_ATLREG_MACHINEWIDE 1)
