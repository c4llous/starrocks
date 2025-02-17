// This file is licensed under the Elastic License 2.0. Copyright 2021 StarRocks Limited.

#include "exec/pipeline/exchange/exchange_source_operator.h"

#include "runtime/data_stream_mgr.h"
#include "runtime/data_stream_recvr.h"
#include "runtime/descriptors.h"
#include "runtime/exec_env.h"
#include "runtime/runtime_state.h"

namespace starrocks::pipeline {
Status ExchangeSourceOperator::prepare(RuntimeState* state) {
    SourceOperator::prepare(state);
    _stream_recvr = std::move(
            static_cast<ExchangeSourceOperatorFactory*>(_factory)->create_stream_recvr(state, _runtime_profile));
    return Status::OK();
}

bool ExchangeSourceOperator::has_output() const {
    return _stream_recvr->has_output();
}

bool ExchangeSourceOperator::is_finished() const {
    return _stream_recvr->is_finished();
}

void ExchangeSourceOperator::set_finishing(RuntimeState* state) {
    _is_finishing = true;
    static_cast<ExchangeSourceOperatorFactory*>(_factory)->close_stream_recvr();
}

StatusOr<vectorized::ChunkPtr> ExchangeSourceOperator::pull_chunk(RuntimeState* state) {
    std::unique_ptr<vectorized::Chunk> chunk = std::make_unique<vectorized::Chunk>();
    RETURN_IF_ERROR(_stream_recvr->get_chunk_for_pipeline(&chunk));
    eval_runtime_bloom_filters(chunk.get());
    return std::move(chunk);
}

std::shared_ptr<DataStreamRecvr> ExchangeSourceOperatorFactory::create_stream_recvr(
        RuntimeState* state, const std::shared_ptr<RuntimeProfile>& profile) {
    if (_stream_recvr != nullptr) {
        return _stream_recvr;
    }
    _stream_recvr = state->exec_env()->stream_mgr()->create_recvr(
            state, _row_desc, state->fragment_instance_id(), _plan_node_id, _num_sender,
            config::exchg_node_buffer_size_bytes, profile, false, nullptr, true, false);
    return _stream_recvr;
}

void ExchangeSourceOperatorFactory::close_stream_recvr() {
    if (--_stream_recvr_cnt == 0) {
        _stream_recvr->close();
    }
}
} // namespace starrocks::pipeline
