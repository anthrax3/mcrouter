/*
 *  Copyright (c) 2017-present, Facebook, Inc.
 *
 *  This source code is licensed under the MIT license found in the LICENSE
 *  file in the root directory of this source tree.
 *
 */

/*
 *  THIS FILE IS AUTOGENERATED. DO NOT MODIFY IT; ALL CHANGES WILL BE LOST IN
 *  VAIN.
 *
 *  @generated
 */
#include "HelloGoodbyeMessages.h"

namespace hellogoodbye {

constexpr const char* const HelloRequest::name;

void HelloRequest::serialize(carbon::CarbonProtocolWriter& writer) const {
  writer.writeStructBegin();
  writer.writeField(1 /* field id */, key());
  writer.writeField(2 /* field id */, shardId());
  writer.writeStructEnd();
  writer.writeStop();
}

void HelloRequest::deserialize(carbon::CarbonProtocolReader& reader) {
  reader.readStructBegin();
  while (true) {
    const auto pr = reader.readFieldHeader();
    const auto fieldType = pr.first;
    const auto fieldId = pr.second;

    if (fieldType == carbon::FieldType::Stop) {
      break;
    }

    switch (fieldId) {
      case 1: {
        reader.readField(key(), fieldType);
        break;
      }
      case 2: {
        reader.readField(shardId(), fieldType);
        break;
      }
      default: {
        reader.skip(fieldType);
        break;
      }
    }
  }
  reader.readStructEnd();
}

void HelloReply::serialize(carbon::CarbonProtocolWriter& writer) const {
  writer.writeStructBegin();
  writer.writeField(1 /* field id */, result());
  writer.writeStructEnd();
  writer.writeStop();
}

void HelloReply::deserialize(carbon::CarbonProtocolReader& reader) {
  reader.readStructBegin();
  while (true) {
    const auto pr = reader.readFieldHeader();
    const auto fieldType = pr.first;
    const auto fieldId = pr.second;

    if (fieldType == carbon::FieldType::Stop) {
      break;
    }

    switch (fieldId) {
      case 1: {
        reader.readField(result(), fieldType);
        break;
      }
      default: {
        reader.skip(fieldType);
        break;
      }
    }
  }
  reader.readStructEnd();
}

constexpr const char* const GoodbyeRequest::name;

void GoodbyeRequest::serialize(carbon::CarbonProtocolWriter& writer) const {
  writer.writeStructBegin();
  writer.writeField(1 /* field id */, key());
  writer.writeField(2 /* field id */, shardId());
  writer.writeStructEnd();
  writer.writeStop();
}

void GoodbyeRequest::deserialize(carbon::CarbonProtocolReader& reader) {
  reader.readStructBegin();
  while (true) {
    const auto pr = reader.readFieldHeader();
    const auto fieldType = pr.first;
    const auto fieldId = pr.second;

    if (fieldType == carbon::FieldType::Stop) {
      break;
    }

    switch (fieldId) {
      case 1: {
        reader.readField(key(), fieldType);
        break;
      }
      case 2: {
        reader.readField(shardId(), fieldType);
        break;
      }
      default: {
        reader.skip(fieldType);
        break;
      }
    }
  }
  reader.readStructEnd();
}

void GoodbyeReply::serialize(carbon::CarbonProtocolWriter& writer) const {
  writer.writeStructBegin();
  writer.writeField(1 /* field id */, result());
  writer.writeField(2 /* field id */, message());
  writer.writeStructEnd();
  writer.writeStop();
}

void GoodbyeReply::deserialize(carbon::CarbonProtocolReader& reader) {
  reader.readStructBegin();
  while (true) {
    const auto pr = reader.readFieldHeader();
    const auto fieldType = pr.first;
    const auto fieldId = pr.second;

    if (fieldType == carbon::FieldType::Stop) {
      break;
    }

    switch (fieldId) {
      case 1: {
        reader.readField(result(), fieldType);
        break;
      }
      case 2: {
        reader.readField(message(), fieldType);
        break;
      }
      default: {
        reader.skip(fieldType);
        break;
      }
    }
  }
  reader.readStructEnd();
}
} // namespace hellogoodbye
