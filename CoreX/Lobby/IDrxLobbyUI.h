// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

<drx3D/CoreX/Lobby/IDrxLobby.h> // <> required for Interfuscator

//! For returning the number of available DLC items in the Online Retail system.
struct SDrxLobbyUIOnlineRetailCounts
{
	u32 newItems;
	u32 totalItems;
};

#if DRX_PLATFORM_ORBIS
	#define DRX_LOBBYUI_CONSUMABLE_OFFER_ID_LENGTH  (16)
	#define DRX_LOBBYUI_CONSUMABLE_OFFER_SKU_LENGTH (4)
	#define DRX_LOBBYUI_CONSUMABLE_ASSET_ID_LENGTH  (16)

struct SStoreOfferID
{
	DrxFixedStringT<DRX_LOBBYUI_CONSUMABLE_OFFER_ID_LENGTH>  productId;
	DrxFixedStringT<DRX_LOBBYUI_CONSUMABLE_OFFER_SKU_LENGTH> skuId;
};

typedef SStoreOfferID                                           TStoreOfferID;
typedef DrxFixedStringT<DRX_LOBBYUI_CONSUMABLE_ASSET_ID_LENGTH> TStoreAssetID;
#else
typedef uint64                                                  TStoreOfferID;
typedef u32                                                  TStoreAssetID;
#endif

struct SDrxLobbyConsumableOfferData
{
	SDrxLobbyConsumableOfferData()
	{
		Clear();
	}

	void Clear()
	{
#if DRX_PLATFORM_ORBIS
		offerID.productId = "";
		offerID.skuId = "";
		assetID = "";
#else
		offerID = 0;
		assetID = 0;
#endif
		purchaseQuantity = 0;
		price.clear();
		name.clear();
		description.clear();
	}

	static const size_t                         k_priceSize = 100;
	static const size_t                         k_nameSize = 100;
	static const size_t                         k_descriptionSize = 420;

	u32                                      purchaseQuantity;
	TStoreOfferID                               offerID;
	TStoreAssetID                               assetID;

	DrxStackStringT<wchar_t, k_priceSize>       price;
	DrxStackStringT<wchar_t, k_nameSize>        name;
	DrxStackStringT<wchar_t, k_descriptionSize> description;
};

struct SDrxLobbyConsumableAssetData
{
	TStoreAssetID assetID;
	u32        assetQuantity;
};

//! \param taskID       Task ID allocated when the function was executed.
//! \param error        Error code - eCLE_Success if the function succeeded or an error that occurred while processing the function.
//! \param pArg         Pointer to application-specified data.
typedef void (* DrxLobbyUICallback)(DrxLobbyTaskID taskID, EDrxLobbyError error, uk pArg);

//! \param taskID       Task ID allocated when the function was executed.
//! \param error        Error code - eCLE_Success if the function succeeded or an error that occurred while processing the function.
//! \param pArg         Pointer to application-specified data.
typedef void (* DrxLobbyUIOnlineRetailStatusCallback)(DrxLobbyTaskID taskID, EDrxLobbyError error, SDrxLobbyUIOnlineRetailCounts* pCounts, uk pArg);

//! DrxLobbyUIGetConsumableOffersCallback.
//! \param taskID        Task ID allocated when the function was executed.
//! \param error         Error code - eCLE_Success if the function succeeded or an error that occurred while processing the function.
//! \param nOffers       Number of offers found.
//! \param pOffers       Array of offer data.
//! \param pArg          Pointer to application-specified data.
typedef void (* DrxLobbyUIGetConsumableOffersCallback)(DrxLobbyTaskID taskID, EDrxLobbyError error, u32 nOffers, SDrxLobbyConsumableOfferData* pOffers, uk pArg);

//! DrxLobbyUIGetConsumableAssetsCallback.
//! \param taskID        Task ID allocated when the function was executed.
//! \param error         Error code - eCLE_Success if the function succeeded or an error that occurred while processing the function.
//! \param nAssets       Number of assets found.
//! \param pAssets       Array of asset data.
//! \param pArg          Pointer to application-specified data.
typedef void (* DrxLobbyUIGetConsumableAssetsCallback)(DrxLobbyTaskID taskID, EDrxLobbyError error, u32 nAssets, SDrxLobbyConsumableAssetData* pAssets, uk pArg);

struct IDrxLobbyUI
{
	// <interfuscator:shuffle>
	virtual ~IDrxLobbyUI(){}

	//! Show the gamer card of the given user id.
	//! \param user         The pad number of the user making the call.
	//! \param userID       The the user id of the gamer card to show.
	//! \param pTaskID      Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb           Callback function that is called when function completes.
	//! \param pCbArg       Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError ShowGamerCard(u32 user, DrxUserID userID, DrxLobbyTaskID* pTaskID, DrxLobbyUICallback cb, uk pCbArg) = 0;

	//! Show the game invite ui to invite users to a session.
	//! \param user         The pad number of the user making the call.
	//! \param h            A handle to the session the invite is for.
	//! \param pUserIDs     Pointer to an array of user ids to send invites to.
	//! \param numUserIDs   The number of users to invite.
	//! \param pTaskID      Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb           Callback function that is called when function completes.
	//! \param pCbArg       Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError ShowGameInvite(u32 user, DrxSessionHandle h, DrxUserID* pUserIDs, u32 numUserIDs, DrxLobbyTaskID* pTaskID, DrxLobbyUICallback cb, uk pCbArg) = 0;

	//! Show the friends ui for the given user.
	//! \param user         The pad number of the user making the call.
	//! \param pTaskID      Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb           Callback function that is called when function completes.
	//! \param pCbArg       Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError ShowFriends(u32 user, DrxLobbyTaskID* pTaskID, DrxLobbyUICallback cb, uk pCbArg) = 0;

	//! Show the friend request ui for inviting a user to be your friend.
	//! \param user          The pad number of the user making the call.
	//! \param userID        The the user id of the user to invite.
	//! \param pTaskID       Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb            Callback function that is called when function completes.
	//! \param pCbArg        Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError ShowFriendRequest(u32 user, DrxUserID userID, DrxLobbyTaskID* pTaskID, DrxLobbyUICallback cb, uk pCbArg) = 0;

	//! Show the Live party ui.
	//! TCR 138 can be satisfied by providing an option labeled "Invite Xbox LIVE Party" and calling ShowParty when a user is in a party with at least one other user.
	//! And providing an option labeled "Invite Friends" and calling ShowFriends if they are not.
	//! \param user        The pad number of the user making the call.
	//! \param pTaskID     Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb          Callback function that is called when function completes.
	//! \param pCbArg      Pointer to application-specified data that is passed to the callback.
	//! \note Xbox Live only.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError ShowParty(u32 user, DrxLobbyTaskID* pTaskID, DrxLobbyUICallback cb, uk pCbArg) = 0;

	//! Show Live Community Sessions ui.
	//! TCR 139 can be satisfied by calling this function.
	//! \param user       The pad number of the user making the call.
	//! \param pTaskID    Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb         Callback function that is called when function completes.
	//! \param pCbArg     Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	//! \note Xbox Live only.
	virtual EDrxLobbyError ShowCommunitySessions(u32 user, DrxLobbyTaskID* pTaskID, DrxLobbyUICallback cb, uk pCbArg) = 0;

	//! Set the Rich Presence string for a user that will be displayed in the platform UI.
	//! For Live a Rich Presence string can be up to 2 lines each of up to 22 characters.
	//! \param user      The pad number of the user making the call.
	//! \param pData     An array of SDrxLobbyUserData used to define the string. The array should start with the main presence string that can contain embedded format tags followed by the items to fill those tags.
	//! \param numData   The number of user data that makes up the string.
	//! \param pTaskID   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb        Callback function that is called when function completes.
	//! \param pCbArg    Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError SetRichPresence(u32 user, SDrxLobbyUserData* pData, u32 numData, DrxLobbyTaskID* pTaskID, DrxLobbyUICallback cb, uk pCbArg) = 0;

	//! Starts the platform UI online retailing process by terminating the game.
	//! \param user         The pad number of the user making the call.
	//! \param pTaskID      Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb           Callback function that is called when function completes.
	//! \param pCbArg       Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError ShowOnlineRetailBrowser(u32 user, DrxLobbyTaskID* pTaskID, DrxLobbyUICallback cb, uk pCbArg) = 0;

	//! Gathers information about new and existing DLC content available on the online retailing service.
	//! Data is returned in the callback in a SDrxLobbyUIOnlineRetailCounts structure.
	//! Worth noting that on PS3 this allocates approximately 1200KB of temporary working buffers for the duration of the task.
	//! \param user        The pad number of the user making the call.
	//! \param pTaskID     Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb          Callback function that is called when function completes.
	//! \param pCbArg      Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError CheckOnlineRetailStatus(u32 user, DrxLobbyTaskID* pTaskID, DrxLobbyUIOnlineRetailStatusCallback cb, uk pCbArg) = 0;

	//! Shows a list of the available messages in the local user's message/mail list.
	//! \param user        The pad number of the user making the call.
	//! \param pTaskID     Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb          Callback function that is called when function completes.
	//! \param pCbArg      Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError ShowMessageList(u32 user, DrxLobbyTaskID* pTaskID, DrxLobbyUICallback cb, uk pCbArg) = 0;

	//! Retrieves a list of consumable offers available.
	//! \param user     The pad number of the user making the call.
	//! \param pTaskID  Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb       Callback function that is called when function completes.
	//! \param pCbArg   Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError GetConsumableOffers(u32 user, DrxLobbyTaskID* pTaskID, const TStoreOfferID* pOfferIDs, u32 offerIdCount, DrxLobbyUIGetConsumableOffersCallback cb, uk pCbArg) = 0;

	//! Shows a UI to confirm/cancel purchasing an offer and start downloading.
	//! \param user      The pad number of the user making the call.
	//! \param offerId   ID of the offer which will be offered for purchase.
	//! \param pTaskID   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb        Callback function that is called when function completes.
	//! \param pCbArg    Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError ShowDownloadOffer(u32 user, TStoreOfferID offerId, DrxLobbyTaskID* pTaskID, DrxLobbyUICallback cb, uk pCbArg) = 0;

	//! Retrieves a list of consumable assets the user owns.
	//! \param user       The pad number of the user making the call.
	//! \param pTaskID    Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb         Callback function that is called when function completes.
	//! \param pCbArg     Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError GetConsumableAssets(u32 user, DrxLobbyTaskID* pTaskID, DrxLobbyUIGetConsumableAssetsCallback cb, uk pCbArg) = 0;

	//! Consumes a quantity of an asset of the passed user so that they no longer have them.
	//! \param user       The pad number of the user making the call.
	//! \param AssetID    The ID of the asset to consume.
	//! \param Quantity   Amount of the asset to consume.
	//! \param pTaskID    Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb         Callback function that is called when function completes.
	//! \param pCbArg     Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError ConsumeAsset(u32 user, TStoreAssetID assetID, u32 quantity, DrxLobbyTaskID* pTaskID, DrxLobbyUICallback cb, uk pCbArg) = 0;
	virtual void           PostLocalizationChecks() = 0;

	//! Cancel the given task. The task will still be running in the background but the callback will not be called when it finishes.
	//! \param taskID     The task to cancel.
	virtual void CancelTask(DrxLobbyTaskID lTaskID) = 0;

	//! PS4 requires recently players be registered with system software, even if they are not friends.
	//! Typically should be called at the end of each game round.
	//! \param user        The pad number of the user making the call.
	//! \param PUserIDs    List of recent players.
	//! \param NumUserIDs  Number of recent players.
	//! \param GameMostStr PS4 only: must supply a string for the type of game played - eg: DEATHMATCH.
	//! \param pTaskID     Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb          Callback function that is called when function completes.
	//! \param pCbArg      Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError AddRecentPlayers(u32 user, DrxUserID* pUserIDs, u32 numUserIDs, tukk gameModeStr, DrxLobbyTaskID* pTaskID, DrxLobbyUICallback cb, uk pCbArg) = 0;

	//! Add a story event to the PS4 activity feed system.
	//! \param user        The pad number of the user making the call.
	//! \param pTaskID     Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb          Callback function that is called when function completes.
	//! \param pCbArg      Pointer to application-specified data that is passed to the callback.
	//! \note Currently PS4 only.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError AddActivityFeed(u32 user, DrxLobbyTaskID* pTaskID, DrxLobbyUICallback cb, uk pCbArg) = 0;

	// </interfuscator:shuffle>
};
