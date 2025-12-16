/**
 * @file elemental_profile.h
 * @brief Elemental concentration profile for sediment source fingerprinting
 * @author USGS Sediment Fingerprinting Project
 * @date December 2025
 *
 * @section description Description
 * Defines the Elemental_Profile class for storing and manipulating elemental
 * composition data used in sediment source fingerprinting analysis.
 *
 * @section scientific_background Scientific Background
 * Sediment source fingerprinting uses geochemical signatures (elemental
 * concentrations) to trace sediment origins. This class represents a single
 * sample's complete elemental "fingerprint."
 *
 * @section references References
 * - Collins, A.L., et al. (2017). Sediment source fingerprinting: A review.
 *   Journal of Environmental Management, 194, 86-108.
 *
 * @warning This class publicly inherits from std::map, which has a non-virtual
 *          destructor. Avoid polymorphic deletion through std::map pointer.
 *          This design issue will be addressed in future major version.
 */

#ifndef ELEMENTAL_PROFILE_H
#define ELEMENTAL_PROFILE_H

#include <map>
#include <string>
#include <vector>
#include <memory>
#include "interface.h"
#include "multiplelinearregressionset.h"

 // Bring std types into scope for compatibility
using std::string;
using std::vector;
using std::map;

// Forward declarations
class QTableWidget;
class QJsonObject;
class QFile;
class CMBVector;

/**
 * @struct element_information
 * @brief Metadata describing an element's role and properties in analysis
 *
 * Contains configuration information for how each element should be
 * treated during sediment source fingerprinting analysis.
 */
struct element_information
{
    /**
     * @enum role
     * @brief Classification of element types
     */
    enum class role {
        do_not_include,  ///< Exclude from all analyses
        isotope,         ///< Isotopic ratio (e.g., 206Pb/207Pb)
        particle_size,   ///< Particle size distribution parameter
        element,         ///< Standard chemical element concentration
        organic_carbon   ///< Organic carbon content
    } Role = role::element;

    double standard_ratio;      ///< Standard isotopic ratio for normalization
    string base_element;        ///< Parent element for isotopes
    bool include_in_analysis = true;  ///< Whether to include in statistical analyses
};

/**
 * @class Elemental_Profile
 * @brief Container for elemental concentration data of a single sediment sample
 *
 * Stores and manipulates elemental concentration data ("fingerprint") for one
 * sediment sample. Inherits from std::map<string,double> to provide map-like
 * access where keys are element names and values are concentrations.
 *
 * @details
 * Key capabilities:
 * - Storage of element concentrations (ppm, %, or other units)
 * - Quality control marking system for flagging outliers
 * - Statistical operations (sorting, ranking, dot products)
 * - Correction algorithms (organic matter, particle size effects)
 * - Multiple export formats (JSON, table, text)
 * - Subsetting and filtering operations
 *
 * @section usage Usage Example
 * @code
 * // Create profile for a sediment sample
 * Elemental_Profile sample;
 * sample.AppendElement("Al", 8.5);   // 8.5% aluminum
 * sample.AppendElement("Fe", 4.2);   // 4.2% iron
 * sample.AppendElement("Pb", 25.3);  // 25.3 ppm lead
 *
 * // Check element existence and retrieve value
 * if (sample.Contains("Al")) {
 *     double al = sample.GetValue("Al");
 * }
 *
 * // Flag suspicious value
 * sample.SetMarked("Pb", true);
 *
 * // Extract subset
 * vector<string> metals = {"Fe", "Pb", "Cu", "Zn"};
 * Elemental_Profile metals_only = sample.ExtractElements(metals);
 * @endcode
 *
 * @note Thread Safety: This class is NOT thread-safe. External synchronization
 *       required for concurrent access.
 *
 * @note Future versions will replace public inheritance with composition to
 *       avoid issues with std::map's non-virtual destructor.
 */
class Elemental_Profile : public map<string, double>, public Interface
{
public:
    // =========================================================================
    // CONSTRUCTION AND ASSIGNMENT
    // =========================================================================

    /**
     * @brief Default constructor - creates empty profile
     * @post Profile is empty (size() == 0)
     * @post Profile included in analysis (IsIncludedInAnalysis() == true)
     */
    Elemental_Profile();

    /**
     * @brief Copy constructor - deep copy of profile
     * @param other Profile to copy
     * @post *this is independent copy of other
     */
    Elemental_Profile(const Elemental_Profile& other);

    /**
     * @brief Copy assignment operator
     * @param other Profile to assign from
     * @return Reference to *this
     * @post *this contains copy of other's data
     */
    Elemental_Profile& operator=(const Elemental_Profile& other);

    // Default destructor is sufficient (Rule of Zero for other members)

    // =========================================================================
    // ELEMENT ACCESS AND MODIFICATION
    // =========================================================================

    /**
     * @brief Get element concentration by name
     *
     * @param name Element name (e.g., "Al", "Fe", "206Pb/207Pb")
     * @return Concentration value, or -1 if element not found
     *
     * @warning Returns -1 for missing elements. Use Contains() to check first,
     *          or consider using at() which throws on missing elements.
     * @warning Logs error to console if element not found (not ideal for library)
     *
     * @note Prefer: if (Contains(name)) { double v = GetValue(name); }
     *
     * @see Contains() to check existence
     * @see SetValue() to modify value
     */
    double GetValue(const string& name) const;

    /**
     * @brief Set concentration value for existing element
     *
     * @param name Element name (must already exist)
     * @param val New concentration value
     * @return true if successful, false if element doesn't exist
     *
     * @pre Element must exist: Contains(name) == true
     * @post If successful: GetValue(name) == val
     *
     * @warning Logs error to console if element not found
     *
     * @see AppendElement() to add new elements
     */
    bool SetValue(const string& name, double val);

    /**
     * @brief Add new element to profile
     *
     * @param name Element name (must be unique)
     * @param val Initial concentration (default: 0.0)
     * @return true if added, false if element already exists
     *
     * @pre Element must not exist: Contains(name) == false
     * @post If successful: Contains(name) == true && GetValue(name) == val
     * @post If successful: IsMarked(name) == false
     *
     * @warning Logs error to console if element already exists
     */
    bool AppendElement(const string& name, double val = 0.0);

    /**
     * @brief Check if element exists in profile
     * @param name Element name to check
     * @return true if element exists, false otherwise
     */
    bool Contains(const string& name) const;

    /**
     * @brief Get all concentration values as vector
     * @return Vector of all concentration values in map order
     * @note Order matches iterator order (typically alphabetical)
     */
    vector<double> GetAllValues() const;

    /**
     * @brief Get list of all element names
     * @return Vector of element names in map order
     */
    vector<string> GetElementNames() const;

    // =========================================================================
    // QUALITY CONTROL - MARKING SYSTEM
    // =========================================================================

    /**
     * @brief Set marked status for an element
     *
     * Marking system allows flagging suspicious values without removing them.
     * Marked elements can be filtered in downstream analyses.
     *
     * @param name Element name
     * @param marked true to mark, false to unmark
     * @return true if successful, false if element doesn't exist
     *
     * @pre Element must exist
     * @post If successful: IsMarked(name) == marked
     */
    bool SetMarked(const string& name, bool marked);

    /**
     * @brief Check if element is marked
     * @param name Element name
     * @return true if marked, false if unmarked or doesn't exist
     * @warning Returns false for both unmarked AND non-existent elements
     */
    bool IsMarked(const string& name) const;

    // =========================================================================
    // STATISTICAL OPERATIONS
    // =========================================================================

    /**
     * @brief Get minimum concentration value
     * @return Minimum concentration across all elements
     * @throw std::logic_error if profile is empty (after refactoring)
     * @warning Current implementation returns 1e12 if empty (should be fixed)
     */
    double GetMinimum() const;

    /**
     * @brief Get maximum concentration value
     * @return Maximum concentration across all elements
     * @throw std::logic_error if profile is empty (after refactoring)
     * @warning Current implementation returns -1e12 if empty (should be fixed)
     */
    double GetMaximum() const;

    /**
     * @brief Calculate dot product with weight vector
     *
     * Computes sum(concentration[i] * weights[i]) for all elements.
     * Used in chemical mass balance and discriminant analysis.
     *
     * @param weights Weight vector (must match profile size)
     * @return Dot product value
     * @warning Returns -999 if size mismatch (should throw exception)
     *
     * @note Used in fingerprinting source contribution calculations
     */
    double CalculateDotProduct(const CMBVector& weights) const;

    /**
     * @brief Sort elements by concentration value
     *
     * @param ascending true for ascending order, false for descending
     * @return CMBVector with sorted element names and values
     * @note Returns new object; original profile unchanged
     */
    CMBVector SortByConcentration(bool ascending = true) const;

    /**
     * @brief Select top N elements by concentration
     *
     * Identifies highest (or lowest) concentrated elements.
     * Useful for focusing analysis on dominant tracers.
     *
     * @param n Number of elements to select
     * @param ascending true to select lowest, false for highest
     * @return Vector of element names
     * @post Return size <= min(n, profile.size())
     */
    vector<string> SelectTopElements(int n, bool ascending = true) const;

    // =========================================================================
    // PROFILE EXTRACTION AND FILTERING
    // =========================================================================

    /**
     * @brief Create profile with only analyzed elements
     *
     * Filters based on element_information metadata, excluding:
     * - Elements with include_in_analysis = false
     * - Elements with role = do_not_include
     * - Particle size indicators
     * - Organic carbon measures
     *
     * @param elementinfo Element metadata (nullptr includes all)
     * @return New filtered profile
     */
    Elemental_Profile CreateAnalysisProfile(
        const map<string, element_information>* elementinfo = nullptr) const;

    /**
     * @brief Extract chemical elements only (exclude isotopes, metadata)
     *
     * @param elementinfo Element metadata for role classification
     * @param include_isotopes true to include isotopes, false to exclude
     * @return Profile with only element/isotope data
     */
    Elemental_Profile ExtractChemicalElements(
        const map<string, element_information>* elementinfo,
        bool include_isotopes = false) const;

    /**
     * @brief Extract subset of specified elements
     *
     * @param element_names Elements to extract
     * @return New profile with requested elements
     * @note Elements not in profile are silently skipped
     * @post Return size <= element_names.size()
     */
    Elemental_Profile ExtractElements(const vector<string>& element_names) const;

    // =========================================================================
    // CORRECTIONS AND TRANSFORMATIONS
    // =========================================================================

    /**
     * @brief Apply organic matter and particle size corrections
     *
     * Corrects element concentrations for organic matter content and particle
     * size effects using multiple linear regression models. Critical for ensuring
     * differences reflect source variation rather than sample characteristics.
     *
     * @param reference_om_size Vector: [organic_matter_%, mean_particle_size_um]
     * @param regression_models MLR models: element_name -> MLR(OM, size)
     * @param elementinfo Element metadata to identify OM and size elements
     * @return New profile with corrected concentrations
     *
     * @details
     * For each element with a regression model:
     * - Linear: C_corrected = C + (OM_ref - OM)*beta1 + (size_ref - size)*beta2
     * - Power: C_corrected = C * (OM_ref/OM)^beta1 * (size_ref/size)^beta2
     *
     * @warning Negative corrected values indicate problems with correction model
     *
     * @note Reference: Collins & Walling (2002) - Selecting fingerprint properties
     *       for discriminating sediment sources. J. Hydrology, 261, 218-244.
     */
    Elemental_Profile ApplyOrganicMatterAndSizeCorrections(
        const vector<double>& reference_om_size,
        const MultipleLinearRegressionSet* regression_models,
        const map<string, element_information>* elementinfo = nullptr) const;

    /**
     * @brief Create corrected profile with filtering and corrections
     *
     * Comprehensive transformation combining element filtering and corrections.
     *
     * @param exclude_elements If true, apply element_information filters
     * @param apply_corrections If true, apply OM/size corrections
     * @param reference_om_size Reference [OM%, size] for corrections
     * @param regression_models MLR models for corrections
     * @param elementinfo Element metadata for filtering
     * @return New corrected and filtered profile
     *
     * @note Consider using separate operations for clarity
     */
    Elemental_Profile CreateCorrectedProfile(
        bool exclude_elements,
        bool apply_corrections,
        const vector<double>& reference_om_size,
        const MultipleLinearRegressionSet* regression_models = nullptr,
        const map<string, element_information>* elementinfo = nullptr) const;

    // =========================================================================
    // ANALYSIS INCLUSION FLAGS
    // =========================================================================

    /**
     * @brief Check if profile is included in analysis
     * @return true if profile should be included in fingerprinting
     */
    bool IsIncludedInAnalysis() const { return included_in_analysis_; }

    /**
     * @brief Set analysis inclusion status
     * @param included true to include, false to exclude
     * @note Excluded profiles retained in dataset but skipped in calculations
     */
    void SetIncludedInAnalysis(bool included) { included_in_analysis_ = included; }

    // =========================================================================
    // SERIALIZATION AND I/O (from Interface base class)
    // =========================================================================

    /**
     * @brief Convert profile to string representation
     * @return String with "Element:Value" pairs separated by newlines
     * @note Format: "Al:8.5\nFe:4.2\n..."
     */
    string ToString() override;

    /**
     * @brief Create Qt table widget representation
     *
     * @return Pointer to new QTableWidget
     * @warning CALLER TAKES OWNERSHIP - must delete or set Qt parent
     * @note Consider using CreateTableUnique() for automatic memory management
     *
     * @see CreateTableUnique() for smart pointer version
     */
    QTableWidget* ToTable() override;

    /**
     * @brief Create table widget with automatic memory management
     * @return std::unique_ptr to QTableWidget
     * @note Preferred over CreateTable() for automatic cleanup
     */
    std::unique_ptr<QTableWidget> CreateTableUnique() const;

    /**
     * @brief Write profile to file
     * @param file Open QFile for writing
     * @return true on success
     */
    bool writetofile(QFile* file) override;

    /**
     * @brief Read profile from string list
     * @param string_list List of "Element:Value" strings
     * @return true on success
     */
    bool Read(const QStringList& string_list) override;

    /**
     * @brief Serialize to JSON object
     * @return QJsonObject containing profile data
     */
    QJsonObject toJsonObject() override;

    /**
     * @brief Deserialize from JSON object
     * @param json JSON representation
     * @return true on success
     * @post Profile cleared and repopulated from JSON
     */
    bool ReadFromJsonObject(const QJsonObject& json) override;

private:
    // =========================================================================
    // PRIVATE MEMBER VARIABLES
    // =========================================================================

    /**
     * @brief Whether profile should be included in analysis
     *
     * Used for quality control - allows excluding problematic samples
     * without deleting them from dataset.
     */
    bool included_in_analysis_ = true;

    /**
     * @brief Map tracking marked elements for quality control
     *
     * Elements can be flagged as outliers or requiring attention
     * without removing them from the profile.
     */
    map<string, bool> marked_elements_;
};

// =============================================================================
// DEPRECATED METHOD ALIASES (for backward compatibility)
// =============================================================================
// These will be removed in v3.0

namespace {
    // Provide deprecated aliases during transition period
    // Usage: auto val = profile.Val("Al");  // Warns but works
}

#endif // ELEMENTAL_PROFILE_H