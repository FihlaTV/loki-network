#include <llarp/dht/context.hpp>
#include <llarp/dht/messages/findrouter.hpp>
#include <llarp/dht/messages/gotrouter.hpp>
#include <llarp/messages/dht.hpp>

#include <router.hpp>

namespace llarp
{
  namespace dht
  {
    bool
    RelayedFindRouterMessage::HandleMessage(
        llarp_dht_context *ctx,
        std::vector< std::unique_ptr< IMessage > > &replies) const
    {
      auto &dht = ctx->impl;
      /// lookup for us, send an immeidate reply
      Key_t us = dht.OurKey();
      if(K == us)
      {
        auto path = dht.router->paths.GetByUpstream(K, pathID);
        if(path)
        {
          replies.emplace_back(
              new GotRouterMessage(K.data(), txid, {dht.router->rc()}, false));
          return true;
        }
        return false;
      }

      Key_t peer;
      Key_t k = K.data();
      // check if we know this in our nodedb first
      RouterContact found;
      if(llarp_nodedb_get_rc(dht.router->nodedb, K, found))
      {
        replies.emplace_back(
            new GotRouterMessage(K.data(), txid, {found}, false));
        return true;
      }
      // lookup if we don't have it in our nodedb
      if(dht.nodes->FindClosest(k, peer))
        dht.LookupRouterForPath(K, txid, pathID, peer);
      return true;
    }

    FindRouterMessage::~FindRouterMessage()
    {
    }

    bool
    FindRouterMessage::BEncode(llarp_buffer_t *buf) const
    {
      if(!bencode_start_dict(buf))
        return false;

      // message type
      if(!bencode_write_bytestring(buf, "A", 1))
        return false;
      if(!bencode_write_bytestring(buf, "R", 1))
        return false;

      // exploritory or not?
      if(!bencode_write_bytestring(buf, "E", 1))
        return false;
      if(!bencode_write_uint64(buf, exploritory ? 1 : 0))
        return false;

      // iterative or not?
      if(!bencode_write_bytestring(buf, "I", 1))
        return false;
      if(!bencode_write_uint64(buf, iterative ? 1 : 0))
        return false;

      // key
      if(!bencode_write_bytestring(buf, "K", 1))
        return false;
      if(!bencode_write_bytestring(buf, K.data(), K.size()))
        return false;

      // txid
      if(!bencode_write_bytestring(buf, "T", 1))
        return false;
      if(!bencode_write_uint64(buf, txid))
        return false;

      // version
      if(!bencode_write_bytestring(buf, "V", 1))
        return false;
      if(!bencode_write_uint64(buf, version))
        return false;

      return bencode_end(buf);
    }

    bool
    FindRouterMessage::DecodeKey(llarp_buffer_t key, llarp_buffer_t *val)
    {
      llarp_buffer_t strbuf;

      if(llarp_buffer_eq(key, "E"))
      {
        uint64_t result;
        if(!bencode_read_integer(val, &result))
          return false;

        exploritory = result != 0;
        return true;
      }

      if(llarp_buffer_eq(key, "I"))
      {
        uint64_t result;
        if(!bencode_read_integer(val, &result))
          return false;

        iterative = result != 0;
        return true;
      }
      if(llarp_buffer_eq(key, "K"))
      {
        if(!bencode_read_string(val, &strbuf))
          return false;
        if(strbuf.sz != K.size())
          return false;

        memcpy(K.data(), strbuf.base, K.size());
        return true;
      }
      if(llarp_buffer_eq(key, "T"))
      {
        return bencode_read_integer(val, &txid);
      }
      if(llarp_buffer_eq(key, "V"))
      {
        return bencode_read_integer(val, &version);
      }
      return false;
    }

    bool
    FindRouterMessage::HandleMessage(
        llarp_dht_context *ctx,
        std::vector< std::unique_ptr< IMessage > > &replies) const
    {
      auto &dht = ctx->impl;
      if(!dht.allowTransit)
      {
        llarp::LogWarn("Got DHT lookup from ", From,
                       " when we are not allowing dht transit");
        return false;
      }
      if(dht.pendingRouterLookups.HasPendingLookupFrom({From, txid}))
      {
        llarp::LogWarn("Duplicate FRM from ", From, " txid=", txid);
        return false;
      }
      RouterContact found;
      if(exploritory)
        return dht.HandleExploritoryRouterLookup(From, txid, K, replies);
      else if(llarp_nodedb_get_rc(dht.router->nodedb, K, found))
      {
        replies.emplace_back(
            new GotRouterMessage(K.data(), txid, {found}, false));
        return true;
      }
      else
        dht.LookupRouterRelayed(From, txid, K.data(), !iterative, replies);
      return true;
    }
  }  // namespace dht
}  // namespace llarp
