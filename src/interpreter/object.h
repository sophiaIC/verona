// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#pragma once

#include "interpreter/bytecode.h"
#include "interpreter/value.h"

#include <verona.h>

namespace verona::interpreter
{
  struct VMDescriptor : public rt::Descriptor
  {
    VMDescriptor(
      std::string_view name,
      size_t method_slots,
      size_t field_slots,
      size_t field_count,
      uint32_t finaliser_slot);

    const std::string name;
    const size_t field_count;
    std::unique_ptr<uint32_t[]> fields;
    std::unique_ptr<uint32_t[]> methods;
    const uint32_t finaliser_slot;
  };

  struct VMObject : public rt::Object
  {
    /**
     * `region` should be the region which contains this object.
     *
     * If the object is in a new region, nullptr should be passed instead.
     */
    explicit VMObject(VMObject* region);

    std::unique_ptr<FieldValue[]> fields;

    const VMDescriptor* descriptor() const
    {
      return static_cast<const VMDescriptor*>(rt::Object::get_descriptor());
    }

    VMObject* region();

    static void trace_fn(const rt::Object* base_object, rt::ObjectStack* stack);
    static void finaliser_fn(rt::Object* base_object);

  private:
    VMObject* parent_;
  };

  struct VMCown : public rt::VCown<VMCown>
  {
    VMObject* contents;

    /**
     * contents should be a region entrypoint. VMCown will take ownership of it.
     */
    explicit VMCown(VMObject* contents) : contents(contents)
    {
      assert((contents == nullptr) || contents->debug_is_iso());
    }

    /**
     * This is for promises., the cown should be initially unscheduled.
     */
    explicit VMCown() : contents(nullptr)
    {
      wake();
    }

    void schedule()
    {
      rt::VCown<VMCown>::schedule();
    }

    void trace(rt::ObjectStack* stack)
    {
      if (contents != nullptr)
        stack->push(contents);
    }

    void trace_possibly_iso(rt::ObjectStack* stack)
    {
      if (contents != nullptr)
        stack->push(contents);
    }
  };
}
