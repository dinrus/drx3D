#pragma once

#include <drx3D/Sys/IDrxPlugin.h>

#ifdef DELETE
#undef DELETE
#endif

namespace Drx
{
	namespace Http
	{
		//! Represents a plug-in that exposes functionality for sending HTTP requests and handling responses
		class IPlugin : public Drx::IEnginePlugin
		{
		public:
			enum class ERequestType
			{
				POST = 0,
				GET,
				PUT,
				PATCH,
				DELETE
			};

			DRXINTERFACE_DECLARE_GUID(IPlugin, "{BC0BA532-EAAD-4B91-AA71-C65E435ABDC1}"_drx_guid);

			virtual ~IPlugin() { }

			enum class EUpdateResult
			{
				Idle = 0,
				ProcessingRequests
			};

			using THeaders = DynArray<std::pair<tukk , tukk >>;

			//! Processes currently on-going HTTP requests, downloading / uploading and triggering callbacks
			//! This can be called as many times as desired even in a single frame, but will also be automatically done once per frame
			//! \return Idle if no requests are being handled, otherwise ProcessingRequests
			virtual EUpdateResult ProcessRequests() = 0;

			typedef std::function<void(tukk szResponse, long responseCode)> TResponseCallback;
			//! Queues a request for sending over the network
			//! \param requestType Determines the type of HTTP request to send
			//! \param szURL null-terminated address to send the request to
			//! \param szBody Request body
			//! \param resultCallback Callback to be invoked when the server has replied
			//! \param headers Array of headers to sent to the server
			//! \returns True if the request is being sent, otherwise false if aborted
			virtual bool Send(ERequestType requestType, tukk szURL, tukk szBody, TResponseCallback resultCallback = nullptr, const THeaders& headers = THeaders()) = 0;
		};
	}
}