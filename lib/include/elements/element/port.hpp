/*=============================================================================
   Copyright (c) 2016-2019 Joel de Guzman

   Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
=============================================================================*/
#if !defined(ELEMENTS_PORT_APRIL_24_2016)
#define ELEMENTS_PORT_APRIL_24_2016

#include <elements/element/proxy.hpp>
#include <memory>

namespace cycfi { namespace elements
{
   ////////////////////////////////////////////////////////////////////////////
   // Port elements
   ////////////////////////////////////////////////////////////////////////////
   class port_base : public proxy_base
   {
   public:

      port_base()
       : _halign(0.0)
       , _valign(0.0)
      {}

      ~port_base() {}

      view_limits             limits(basic_context const& ctx) const override;
      void                    prepare_subject(context& ctx) override;
      void                    draw(context const& ctx) override;

      double                  halign() const { return _halign; }
      void                    halign(double val) { _halign = val; }
      double                  valign() const { return _valign; }
      void                    valign(double val) { _valign = val; }

   private:

      double                  _halign;
      double                  _valign;
   };

   template <typename Subject>
   inline proxy<Subject, port_base>
   port(Subject&& subject)
   {
      return { std::forward<Subject>(subject) };
   }

   class vport_base : public proxy_base
   {
   public:

      vport_base()
       : _valign(0.0)
      {}

      ~vport_base() {}

      view_limits             limits(basic_context const& ctx) const override;
      void                    prepare_subject(context& ctx) override;
      void                    draw(context const& ctx) override;

      double                  valign() const { return _valign; }
      void                    valign(double val) { _valign = val; }

   private:

      double                  _valign;
   };

   template <typename Subject>
   inline proxy<Subject, vport_base>
   vport(Subject&& subject)
   {
      return { std::forward<Subject>(subject) };
   }

   ////////////////////////////////////////////////////////////////////////////
   // Scrollers
   ////////////////////////////////////////////////////////////////////////////

   // scrollable: Mixin class for a element that is scrollable
   // scroll the rectangle, r into view
   class scrollable
   {
   public:
      virtual ~scrollable() = default;

      struct scrollable_context
      {
         context const* context_ptr;
         scrollable*    scrollable_ptr;

         // scroll the rectangle, r into view
         bool scroll_into_view(rect r_)
         {
            if (scrollable_ptr && context_ptr)
            {
               rect r = r_;
               return scrollable_ptr->scroll_into_view(*context_ptr, r);
            }
            return false;
         }
      };

      virtual bool               scroll_into_view(context const& ctx, rect r) = 0;
      static scrollable_context  find(context const& ctx);
   };

   enum
   {
      no_scrollbars  = 1,
      no_hscroll     = 1 << 1,
      no_vscroll     = 1 << 2
   };

   // Base proxy class for views that are scrollable
   class scroller_base : public port_base, public scrollable
   {
   public:

      static float            width;

      scroller_base(int traits = 0)
       : _tracking(none)
       , _traits(traits)
      {}

      ~scroller_base() {}

      view_limits             limits(basic_context const& ctx) const override;
      element*                hit_test(context const& ctx, point p) override;
      void                    draw(context const& ctx) override;

      element*                click(context const& ctx, mouse_button btn) override;
      void                    drag(context const& ctx, mouse_button btn) override;
      bool                    scroll(context const& ctx, point dir, point p) override;
      bool                    scroll_into_view(context const& ctx, rect r) override;
      bool                    cursor(context const& ctx, point p, cursor_tracking status) override;
      bool                    key(context const& ctx, key_info k) override;
      bool                    is_control() const override;

      struct scrollbar_info
      {
         double   pos;
         float    extent;
         rect     bounds;
      };

      virtual void            draw_scroll_bar(context const& ctx, scrollbar_info const& info, point mp);
      virtual rect            scroll_bar_position(context const& ctx, scrollbar_info const& info);

   private:

      struct scrollbar_bounds
      {
         rect vscroll_bounds;
         rect hscroll_bounds;
         bool has_v = false;
         bool has_h = false;
      };

      enum tracking_status
      {
         none,
         start,
         tracking_v,
         tracking_h
      };

      scrollbar_bounds  get_scrollbar_bounds(context const& ctx);
      bool              reposition(context const& ctx, point p);

      bool              has_scrollbars() const { return !(_traits & no_scrollbars); }
      bool              allow_hscroll() const { return !(_traits & no_hscroll); }
      bool              allow_vscroll() const { return !(_traits & no_vscroll); }

      point             _offset;
      tracking_status   _tracking;
      int               _traits;
   };

   template <typename Subject>
   inline proxy<Subject, scroller_base>
   scroller(Subject&& subject, int traits = 0)
   {
      return { std::forward<Subject>(subject), traits };
   }

   template <typename Subject>
   inline proxy<Subject, scroller_base>
   vscroller(Subject&& subject, int traits = 0)
   {
      return { std::forward<Subject>(subject), traits | no_hscroll };
   }

   template <typename Subject>
   inline proxy<Subject, scroller_base>
   hscroller(Subject&& subject, int traits = 0)
   {
      return { std::forward<Subject>(subject), traits | no_vscroll };
   }
}}

#endif
