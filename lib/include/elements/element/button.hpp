/*=============================================================================
   Copyright (c) 2016-2023 Joel de Guzman

   Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
=============================================================================*/
#if !defined(ELEMENTS_BUTTON_APRIL_21_2016)
#define ELEMENTS_BUTTON_APRIL_21_2016

#include <elements/element/layer.hpp>
#include <elements/element/proxy.hpp>
#include <elements/element/selection.hpp>
#include <elements/element/traversal.hpp>
#include <elements/support/context.hpp>
#include <elements/support/sender.hpp>
#include <elements/view.hpp>
#include <type_traits>
#include <concepts>

namespace cycfi::elements
{
   ////////////////////////////////////////////////////////////////////////////
   // Basic Button
   ////////////////////////////////////////////////////////////////////////////
   namespace concepts
   {
      template <typename T>
      concept ButtonCallback = requires(T f)
      {
         { f(true) } -> std::same_as<void>;
      };
   }

   /**
    *  \struct button_state
    *
    *  \brief A structure to maintain and manage the state of a button.
    *
    *  This structure captures the various states that a button can have:
    *  - `value`: The button's value; 0(off) or 1(on).
    *  - `hilite`: True if the button is highlighted.
    *  - `tracking`: True if the mouse button being pressed.
    *  - `enabled`: True if the button is enabled.
    */
   struct button_state
   {
                        button_state()
                         : value{false}
                         , hilite{false}
                         , tracking{false}
                         , enabled{true}
                        {}

      bool              value : 1;
      bool              hilite : 1;
      bool              tracking : 1;
      bool              enabled : 1;
   };

   /**
    *  \class basic_button
    *
    *  \brief
    *    A class that represents a basic GUI button, acting as a proxy which
    *    delegates the rendering to a "button styler".
    *
    *    The `basic_button` class is a foundational class for creating a GUI
    *    button. This class is a proxy which delegates the rendering of the
    *    actual button to a button styler subject. This division of
    *    responsibilities allows for more flexibility in dictating the
    *    button's appearance and interaction. The `basic_button` class
    *    handles user interactions, while the button styler manages the
    *    button's visual presentation. With this pattern, different stylers
    *    can be implemented for various visual representations, for instance,
    *    plain buttons, radio buttons, slide switches, checkboxes, and more.
    *
    *    The communication with the "button styler" is done via the
    *    `receiver<button_state>` API. This API provides a means for the
    *    `basic_button` to update the button styler about changes in button's
    *    state (such as on/off value, highlight, enabled state etc.) enabling
    *    the styler to adjust the visual representation accordingly.
    */
   class basic_button : public proxy_base, public receiver<bool>
   {
   public:

      using button_function = std::function<void(bool)>;

      bool              wants_control() const override;
      bool              click(context const& ctx, mouse_button btn) override;
      bool              cursor(context const& ctx, point p, cursor_tracking status) override;
      void              drag(context const& ctx, mouse_button btn) override;
      element*          hit_test(context const& ctx, point p, bool leaf, bool control) override;

      void              enable(bool state = true) override;
      bool              is_enabled() const override;

      void              value(bool val) override;
      bool              value() const override  { return _state.value; }
      bool              tracking() const        { return _state.tracking; }
      bool              hilite() const          { return _state.hilite; }

      void              edit(view& view_, bool val) override;

      button_function   on_click;

   protected:

      bool              state(bool val);
      void              tracking(bool val);
      void              hilite(bool val);

   private:

      bool              update_receiver();

      button_state      _state;
   };

   ////////////////////////////////////////////////////////////////////////////
   // Toggle Button
   ////////////////////////////////////////////////////////////////////////////
   template <typename Base>
   class basic_toggle_button : public Base
   {
   public:

      using Base::Base;

      bool              click(context const& ctx, mouse_button btn) override;
      void              drag(context const& ctx, mouse_button btn) override;

   private:

      bool              _current_state;
   };

   template <typename Base>
   inline bool basic_toggle_button<Base>::click(context const& ctx, mouse_button btn)
   {
      if (!ctx.enabled || !this->is_enabled())
         return false;

      if (btn.state != mouse_button::left || !ctx.bounds.includes(btn.pos))
      {
         this->tracking(false);
         ctx.view.refresh(ctx);
         return false;
      }

      if (btn.down)
      {
         this->tracking(true);
         if (this->state(!this->value()))    // toggle the state
         {
            ctx.view.refresh(ctx);           // we need to save the current state, the state
            _current_state = this->value();  // can change in the drag function and so we'll
         }                                   // need it later when the button is finally released
      }
      else
      {
         this->tracking(false);
         this->state(_current_state);
         if (this->on_click)
            this->on_click(this->value());
         ctx.view.refresh(ctx);
      }
      return true;
   }

   template <typename Base>
   inline void basic_toggle_button<Base>::drag(context const& ctx, mouse_button btn)
   {
      this->hilite(ctx.bounds.includes(btn.pos));
      if (this->state(!_current_state ^ ctx.bounds.includes(btn.pos)))
         ctx.view.refresh(ctx);
   }

   ////////////////////////////////////////////////////////////////////////////
   // Latching Button
   ////////////////////////////////////////////////////////////////////////////
   template <typename Base>
   class basic_latching_button : public Base
   {
   public:

      using Base::Base;

      bool              click(context const& ctx, mouse_button btn) override;
   };

   template <typename Base>
   inline bool basic_latching_button<Base>::click(context const& ctx, mouse_button btn)
   {
      if (btn.down && this->value())
         return false;

      if (btn.state != mouse_button::left || !ctx.bounds.includes(btn.pos))
      {
         this->tracking(false);
         ctx.view.refresh(ctx);
         return false;
      }

      if (btn.down)
      {
         this->tracking(true);
         this->on_tracking(ctx, this->begin_tracking);
      }
      else
      {
         this->tracking(false);
         this->on_tracking(ctx, this->end_tracking);
         if (this->on_click)
            this->on_click(true);
         ctx.view.refresh(ctx);
      }
      if (btn.down && this->state(ctx.bounds.includes(btn.pos)))
         ctx.view.refresh(ctx);
      return true;
   }

   ////////////////////////////////////////////////////////////////////////////
   // Basic Choice
   ////////////////////////////////////////////////////////////////////////////
   template <typename Base>
   class basic_choice : public basic_latching_button<Base>, public selectable
   {
   public:

      using basic_latching_button<Base>::basic_latching_button;

      void              select(bool state) override;
      bool              is_selected() const override;
      bool              click(context const& ctx, mouse_button btn) override;

   private:

      friend void       basic_choice_click(context const& ctx, selectable& s);
   };

   template <typename Base>
   inline bool basic_choice<Base>::is_selected() const
   {
      return this->value();
   }

   template <typename Base>
   inline void basic_choice<Base>::select(bool state)
   {
      if (state != is_selected())
         this->value(state);
   }

   template <typename Base>
   bool basic_choice<Base>::click(context const& ctx, mouse_button btn)
   {
      if (btn.state == mouse_button::left)
      {
         if (btn.down)
         {
            return basic_latching_button<Base>::click(ctx, btn);
         }
         else
         {
            auto r = basic_latching_button<Base>::click(ctx, btn);
            if (this->value())
               basic_choice_click(ctx, *this);
            return r;
         }
      }
      return false;
   }

   template <concepts::Element Subject>
   inline basic_choice<proxy<Subject, basic_button>>
   choice(Subject&& subject)
   {
      return {std::forward<Subject>(subject)};
   }
}

#endif
