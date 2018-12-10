#ifndef LLARP_RELAY_COMMIT_HPP
#define LLARP_RELAY_COMMIT_HPP
#include <llarp/crypto.hpp>
#include <llarp/encrypted_ack.hpp>
#include <llarp/encrypted_frame.hpp>
#include <llarp/link_message.hpp>
#include <llarp/path_types.hpp>
#include <llarp/pow.hpp>
#include <array>

namespace llarp
{
  // forward declare
  namespace path
  {
    struct PathContext;
  }

  struct LR_CommitRecord
  {
    PubKey commkey;
    RouterID nextHop;
    TunnelNonce tunnelNonce;
    PathID_t txid, rxid;

    PoW *work         = nullptr;
    uint64_t version  = 0;
    uint64_t lifetime = 0;

    bool
    BDecode(llarp_buffer_t *buf);

    bool
    BEncode(llarp_buffer_t *buf) const;

    ~LR_CommitRecord();

    bool
    operator==(const LR_CommitRecord &other) const;

   private:
    static bool
    OnKey(dict_reader *r, llarp_buffer_t *buf);
  };

  struct LR_CommitMessage : public ILinkMessage
  {
    std::array< EncryptedFrame, 8 > frames;

    LR_CommitMessage() : ILinkMessage()
    {
    }

    LR_CommitMessage(ILinkSession *from) : ILinkMessage(from)
    {
    }

    ~LR_CommitMessage();

    void
    Clear();

    bool
    DecodeKey(llarp_buffer_t key, llarp_buffer_t *buf);

    bool
    BEncode(llarp_buffer_t *buf) const;

    bool
    HandleMessage(llarp::Router *router) const;

    bool
    AsyncDecrypt(llarp::path::PathContext *context) const;
  };
}  // namespace llarp

#endif
