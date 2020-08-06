#ifndef slic3r_PresetComboBoxes_hpp_
#define slic3r_PresetComboBoxes_hpp_

#include <wx/bmpcbox.h>
#include <wx/gdicmn.h>

#include "libslic3r/Preset.hpp"
#include "wxExtensions.hpp"
#include "GUI_Utils.hpp"

class wxString;
class wxTextCtrl;
class wxStaticText;
class ScalableButton;
class wxBoxSizer;
class wxComboBox;
class wxStaticBitmap;

namespace Slic3r {

namespace GUI {

class BitmapCache;

// ---------------------------------
// ***  PresetComboBox  ***
// ---------------------------------

// BitmapComboBox used to presets list on Sidebar and Tabs
class PresetComboBox : public wxBitmapComboBox
{
public:
    PresetComboBox(wxWindow* parent, Preset::Type preset_type, const wxSize& size = wxDefaultSize);
    ~PresetComboBox();

	enum LabelItemType {
		LABEL_ITEM_PHYSICAL_PRINTER = 0xffffff01,
		LABEL_ITEM_DISABLED,
		LABEL_ITEM_MARKER,
		LABEL_ITEM_PHYSICAL_PRINTERS,
		LABEL_ITEM_WIZARD_PRINTERS,
        LABEL_ITEM_WIZARD_FILAMENTS,
        LABEL_ITEM_WIZARD_MATERIALS,

        LABEL_ITEM_MAX,
	};

    void set_label_marker(int item, LabelItemType label_item_type = LABEL_ITEM_MARKER);
    bool set_printer_technology(PrinterTechnology pt);

    void set_selection_changed_function(std::function<void(int)> sel_changed) { on_selection_changed = sel_changed; }

    bool is_selected_physical_printer();

    // Return true, if physical printer was selected 
    // and next internal selection was accomplished
    bool selection_is_changed_according_to_physical_printers();

    void update(std::string select_preset);

    virtual void update();
    virtual void msw_rescale();

protected:
    typedef std::size_t Marker;
    std::function<void(int)>    on_selection_changed { nullptr };

    Preset::Type        m_type;
    std::string         m_main_bitmap_name;

    PresetBundle*       m_preset_bundle {nullptr};
    PresetCollection*   m_collection {nullptr};

    // Caching bitmaps for the all bitmaps, used in preset comboboxes
    static BitmapCache& bitmap_cache();

    // Indicator, that the preset is compatible with the selected printer.
    ScalableBitmap      m_bitmapCompatible;
    // Indicator, that the preset is NOT compatible with the selected printer.
    ScalableBitmap      m_bitmapIncompatible;

    int m_last_selected;
    int m_em_unit;

    // parameters for an icon's drawing
    int icon_height;
    int norm_icon_width;
    int thin_icon_width;
    int wide_icon_width;
    int space_icon_width;
    int thin_space_icon_width;
    int wide_space_icon_width;

    PrinterTechnology printer_technology {ptAny};

    void invalidate_selection();
    void validate_selection(bool predicate = false);
    void update_selection();

#ifdef __linux__
    static const char* separator_head() { return "------- "; }
    static const char* separator_tail() { return " -------"; }
#else // __linux__ 
    static const char* separator_head() { return "————— "; }
    static const char* separator_tail() { return " —————"; }
#endif // __linux__
    static wxString    separator(const std::string& label);

    wxBitmap* get_bmp(  std::string bitmap_key, bool wide_icons, const std::string& main_icon_name, 
                        bool is_compatible = true, bool is_system = false, bool is_single_bar = false,
                        std::string filament_rgb = "", std::string extruder_rgb = "");

    wxBitmap* get_bmp(  std::string bitmap_key, const std::string& main_icon_name, const std::string& next_icon_name,
                        bool is_enabled = true, bool is_compatible = true, bool is_system = false);

#ifdef __APPLE__
    /* For PresetComboBox we use bitmaps that are created from images that are already scaled appropriately for Retina
     * (Contrary to the intuition, the `scale` argument for Bitmap's constructor doesn't mean
     * "please scale this to such and such" but rather
     * "the wxImage is already sized for backing scale such and such". )
     * Unfortunately, the constructor changes the size of wxBitmap too.
     * Thus We need to use unscaled size value for bitmaps that we use
     * to avoid scaled size of control items.
     * For this purpose control drawing methods and
     * control size calculation methods (virtual) are overridden.
     **/
    virtual bool OnAddBitmap(const wxBitmap& bitmap) override;
    virtual void OnDrawItem(wxDC& dc, const wxRect& rect, int item, int flags) const override;
#endif

private:
    void fill_width_height();
};


// ---------------------------------
// ***  PlaterPresetComboBox  ***
// ---------------------------------

class PlaterPresetComboBox : public PresetComboBox
{
public:
    PlaterPresetComboBox(wxWindow *parent, Preset::Type preset_type);
    ~PlaterPresetComboBox();

    ScalableButton* edit_btn { nullptr };

    void set_extruder_idx(const int extr_idx)   { m_extruder_idx = extr_idx; }
    int  get_extruder_idx() const               { return m_extruder_idx; }

    bool switch_to_tab();
    void show_add_menu();
    void show_edit_menu();

    void update() override;
    void msw_rescale() override;

private:
    int     m_extruder_idx = -1;
};


// ---------------------------------
// ***  TabPresetComboBox  ***
// ---------------------------------

class TabPresetComboBox : public PresetComboBox
{
    bool show_incompatible {false};
    bool m_enable_all {false};

public:
    TabPresetComboBox(wxWindow *parent, Preset::Type preset_type);
    ~TabPresetComboBox() {}
    void set_show_incompatible_presets(bool show_incompatible_presets) {
        show_incompatible = show_incompatible_presets;
    }

    void update() override;
    void update_dirty();
    void msw_rescale() override;

    void set_enable_all(bool enable=true) { m_enable_all = enable; }

    PresetCollection*   presets()   const { return m_collection; }
    Preset::Type        type()      const { return m_type; }
};


//------------------------------------------------
//          SavePresetDialog
//------------------------------------------------

class SavePresetDialog : public DPIDialog
{
    enum ActionType
    {
        ChangePreset,
        AddPreset,
        Switch, 
        UndefAction
    };

    struct Item
    {
        enum ValidationType
        {
            Valid,
            NoValid,
            Warning
        };

        Item(Preset::Type type, const std::string& suffix, wxBoxSizer* sizer, SavePresetDialog* parent);

        void            update_valid_bmp();
        void            accept();

        bool            is_valid()      const { return m_valid_type != NoValid; }
        Preset::Type    type()          const { return m_type; }
        std::string     preset_name()   const { return m_preset_name; }

    private:
        Preset::Type    m_type;
        ValidationType  m_valid_type;
        std::string		m_preset_name;

        SavePresetDialog*   m_parent        {nullptr};
        wxStaticBitmap*     m_valid_bmp     {nullptr};
        wxComboBox*         m_combo         {nullptr};
        wxStaticText*       m_valid_label   {nullptr};

        PresetCollection*   m_presets       {nullptr};

        void update();
    };

    std::vector<Item>   m_items;

    wxBoxSizer*         m_presets_sizer     {nullptr};
    wxStaticText*       m_label             {nullptr};
    wxRadioBox*         m_action_radio_box  {nullptr};
    wxBoxSizer*         m_radio_sizer       {nullptr};  
    ActionType          m_action            {UndefAction};

    std::string         m_ph_printer_name;
    std::string         m_old_preset_name;

public:

    SavePresetDialog(Preset::Type type, const std::string& suffix);
    ~SavePresetDialog() {}

    void AddItem(Preset::Type type, const std::string& suffix);

    std::string get_name();
    std::string get_name(Preset::Type type);

    bool enable_ok_btn() const;
    void add_info_for_edit_ph_printer(wxBoxSizer *sizer);
    void update_info_for_edit_ph_printer(const std::string &preset_name);
    void layout();

protected:
    void on_dpi_changed(const wxRect& suggested_rect) override;
    void on_sys_color_changed() override {}

private:
    void update_physical_printers(const std::string& preset_name);
    void accept();
};

} // namespace GUI
} // namespace Slic3r

#endif
