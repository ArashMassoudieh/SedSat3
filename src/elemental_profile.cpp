
/**
 * @file elemental_profile.cpp
 * @brief Implementation of Elemental_Profile class
 */

#include "elemental_profile.h"
#include <iostream>
#include "Utilities.h"
#include <QStringList>
#include <QFile>


// =============================================================================
// CONSTRUCTION AND ASSIGNMENT
// =============================================================================

/**
* @brief Default constructor - creates empty elemental profile
*
*Initializes an empty profile with no elements.The profile is marked as
* included in analysis by default.
*
*@post size() == 0
* @post IsIncludedInAnalysis() == true
* @post No elements are marked
*
*@note Uses member initializer list for efficiency
**/
Elemental_Profile::Elemental_Profile()
    : map<string, double>(),
    Interface(),
    included_in_analysis_(true),
    marked_elements_()
{
    // All initialization done in initializer list
    // Empty body is fine - demonstrates proper C++ style
}


/**
 * @brief Copy constructor - creates deep copy of profile
 *
 * Creates an independent copy of another profile, including all element
 * concentrations, marked status flags, and analysis inclusion setting.
 *
 * @param other Profile to copy from
 *
 * @post *this contains all data from other
 * @post size() == other.size()
 * @post IsIncludedInAnalysis() == other.IsIncludedInAnalysis()
 * @post Modifications to *this do not affect other
 *
 * @note Copy constructor calls base class copy constructors first,
 *       then copies member variables
 */
Elemental_Profile::Elemental_Profile(const Elemental_Profile& other)
    : map<string, double>(other),  // Copy base class (element concentrations)
    Interface(other),              // Copy Interface base class
    included_in_analysis_(other.included_in_analysis_),
    marked_elements_(other.marked_elements_)
{
    // All copying done in initializer list - efficient and exception-safe
}


/*/
*@brief Copy assignment operator
*
* Assigns the contents of another profile to this one, replacing all
* existing data.Handles self - assignment safely.
*
*@param other Profile to assign from
* @return Reference to * this for chaining(e.g., a = b = c)
*
* @post* this contains copy of other's data
* @post Previous contents of * this are replaced
* @post Self - assignment(a = a) is safe
*
*@note Assignment order is important : Interface first, then members,
* then base class to ensure proper cleanup if exceptions occur
*/
Elemental_Profile & Elemental_Profile::operator=(const Elemental_Profile & other)
{
    // Check for self-assignment (important for performance and correctness)
    if (this == &other) {
        return *this;
    }

    // Assign in proper order:
    // 1. Interface base class
    Interface::operator=(other);

    // 2. Member variables
    included_in_analysis_ = other.included_in_analysis_;
    marked_elements_ = other.marked_elements_;

    // 3. map base class (element concentrations)
    //    Done last because it might throw, and we want other data copied first
    map<string, double>::operator=(other);

    return *this;
}

/**
 * @brief Create profile containing only elements included in analysis
 *
 * Filters the profile based on element_information metadata, keeping only
 * elements that should be included in statistical analysis. Excludes:
 * - Elements with include_in_analysis = false
 * - Elements with role = do_not_include
 * - Particle size indicators (role = particle_size)
 * - Organic carbon measurements (role = organic_carbon)
 *
 * The marked status of elements is preserved in the filtered profile.
 *
 * @param element_info Pointer to element metadata map (nullptr to include all)
 * @return New profile containing only analysis-appropriate elements
 *
 * @post If element_info == nullptr: returns copy of entire profile
 * @post If element_info != nullptr: returns filtered subset
 * @post Marked status preserved for included elements
 * @post IsIncludedInAnalysis() state copied to output
 *
 * @note Original profile is unchanged (const method)
 *
 * @example
 * @code
 * Elemental_Profile full_profile;
 * full_profile.AppendElement("Al", 8.5);
 * full_profile.AppendElement("OC", 2.5);  // Organic carbon
 * full_profile.AppendElement("D50", 125); // Particle size
 *
 * map<string, element_information> info;
 * info["Al"].Role = element_information::role::element;
 * info["Al"].include_in_analysis = true;
 * info["OC"].Role = element_information::role::organic_carbon;
 * info["D50"].Role = element_information::role::particle_size;
 *
 * // Get only analysis elements
 * Elemental_Profile analysis = full_profile.CreateAnalysisProfile(&info);
 * // analysis contains only "Al", not "OC" or "D50"
 * @endcode
 */
Elemental_Profile Elemental_Profile::CreateAnalysisProfile(
    const map<string, element_information>* element_info) const
{
    // If no metadata provided, return complete copy
    if (element_info == nullptr) {
        return *this;
    }

    // Create filtered profile
    Elemental_Profile filtered_profile;

    // Copy analysis inclusion flag
    filtered_profile.included_in_analysis_ = this->included_in_analysis_;

    // Iterate through all elements in current profile
    for (const auto& element_entry : *this) {
        const string& element_name = element_entry.first;
        const double concentration = element_entry.second;

        // Check if element exists in metadata
        auto info_iter = element_info->find(element_name);
        if (info_iter == element_info->end()) {
            // Element not in metadata - skip it
            continue;
        }

        const element_information& info = info_iter->second;

        // Apply inclusion criteria
        bool should_include = info.include_in_analysis &&
            info.Role != element_information::role::do_not_include &&
            info.Role != element_information::role::particle_size &&
            info.Role != element_information::role::organic_carbon;

        if (should_include) {
            // Add element to filtered profile
            filtered_profile[element_name] = concentration;

            // Preserve marked status if element was marked
            auto marked_iter = marked_elements_.find(element_name);
            if (marked_iter != marked_elements_.end() && marked_iter->second) {
                filtered_profile.marked_elements_[element_name] = true;
            }
        }
    }

    return filtered_profile;
}

/**
 * @brief Create corrected and filtered profile
 *
 * Creates a new profile by applying organic matter/particle size corrections
 * and/or filtering elements based on metadata. This method combines two
 * optional operations:
 * 1. Correction for organic matter and particle size effects
 * 2. Filtering to include only analysis-appropriate elements
 *
 * @param exclude_elements If true, filter based on element_info metadata
 * @param apply_corrections If true, apply OM/size corrections using MLR
 * @param reference_om_size Reference values [organic_matter_%, particle_size_um]
 * @param regression_models Pointer to MLR models for corrections (required if apply_corrections=true)
 * @param element_info Pointer to element metadata (required if exclude_elements=true)
 *
 * @return New profile with corrections and/or filtering applied
 *
 * @pre If apply_corrections=true, regression_models must not be nullptr
 * @pre If exclude_elements=true, element_info must not be nullptr
 * @pre reference_om_size should have 2 elements: [OM%, size]
 *
 * @post Original profile unchanged (const method)
 * @post If both flags false, returns copy of original profile
 *
 * @warning Original implementation had bug: correction applied repeatedly in loop!
 *
 * @note Consider using separate methods for clarity:
 *       - profile.ApplyOrganicMatterAndSizeCorrections(...).CreateAnalysisProfile(...)
 *
 * @example
 * @code
 * Elemental_Profile raw_profile;
 * // ... populate profile ...
 *
 * vector<double> reference = {2.5, 125.0};  // 2.5% OM, 125 um
 *
 * // Apply both corrections and filtering
 * Elemental_Profile corrected = raw_profile.CreateCorrectedProfile(
 *     true,           // exclude_elements
 *     true,           // apply_corrections
 *     reference,      // reference values
 *     &mlr_models,    // regression models
 *     &element_info   // metadata
 * );
 * @endcode
 */
Elemental_Profile Elemental_Profile::CreateCorrectedProfile(
    bool exclude_elements,
    bool apply_corrections,
    const vector<double>& reference_om_size,
    const MultipleLinearRegressionSet* regression_models,
    const map<string, element_information>* element_info) const
{
    // Start with current profile
    Elemental_Profile result = *this;

    // Step 1: Apply OM/size corrections if requested
    if (apply_corrections) {
        if (regression_models == nullptr) {
            // Log warning or throw exception in production code
            std::cerr << "Warning: apply_corrections=true but regression_models is null. "
                << "Skipping corrections." << std::endl;
        }
        else {
            result = result.ApplyOrganicMatterAndSizeCorrections(
                reference_om_size,
                regression_models,
                element_info
            );
        }
    }

    // Step 2: Filter elements if requested
    if (exclude_elements) {
        if (element_info == nullptr) {
            // Log warning or throw exception in production code
            std::cerr << "Warning: exclude_elements=true but element_info is null. "
                << "Skipping filtering." << std::endl;
        }
        else {
            result = result.CreateAnalysisProfile(element_info);
        }
    }

    return result;
}

/**
 * @brief Extract only chemical elements and optionally isotopes
 *
 * Creates a new profile containing only true chemical elements, filtering out
 * metadata fields such as particle size, organic carbon content, and explicitly
 * excluded elements. Optionally includes isotopic ratios.
 *
 * This is useful when you want to perform statistical analysis on only the
 * geochemical tracers, excluding auxiliary measurements.
 *
 * @param element_info Pointer to element metadata map (must not be null)
 * @param include_isotopes If true, include isotopic ratios; if false, exclude them
 *
 * @return New profile containing only chemical elements (and isotopes if requested)
 *
 * @pre element_info must not be nullptr
 * @post Original profile unchanged (const method)
 * @post Result contains only elements where:
 *       - Role == element, OR
 *       - Role == isotope AND include_isotopes == true
 *
 * @throw std::invalid_argument if element_info is nullptr (after refactoring)
 * @warning Current implementation crashes if element_info is null or element not in metadata
 *
 * @note Elements not present in element_info are silently skipped
 *
 * @example
 * @code
 * Elemental_Profile full_profile;
 * full_profile.AppendElement("Al", 8.5);           // element
 * full_profile.AppendElement("206Pb/207Pb", 1.2);  // isotope
 * full_profile.AppendElement("OC", 2.5);           // organic carbon
 * full_profile.AppendElement("D50", 125);          // particle size
 *
 * map<string, element_information> info;
 * info["Al"].Role = element_information::role::element;
 * info["206Pb/207Pb"].Role = element_information::role::isotope;
 * info["OC"].Role = element_information::role::organic_carbon;
 * info["D50"].Role = element_information::role::particle_size;
 *
 * // Get only elements (no isotopes)
 * Elemental_Profile elements_only = full_profile.ExtractChemicalElements(&info, false);
 * // Result: contains only "Al"
 *
 * // Get elements and isotopes
 * Elemental_Profile with_isotopes = full_profile.ExtractChemicalElements(&info, true);
 * // Result: contains "Al" and "206Pb/207Pb"
 * @endcode
 */
Elemental_Profile Elemental_Profile::ExtractChemicalElements(
    const map<string, element_information>* element_info,
    bool include_isotopes) const
{
    // Validate input - element_info is required
    if (element_info == nullptr) {
        // For now, log error and return empty profile
        // Future: throw std::invalid_argument("element_info cannot be null")
        std::cerr << "Error: ExtractChemicalElements called with null element_info. "
            << "Returning empty profile." << std::endl;
        return Elemental_Profile();
    }

    Elemental_Profile extracted_profile;

    // Iterate through all elements in current profile
    for (const auto& [element_name, concentration] : *this) {

        // Check if element exists in metadata
        auto info_iter = element_info->find(element_name);
        if (info_iter == element_info->end()) {
            // Element not in metadata - skip it
            // Could log warning: std::cerr << "Warning: Element '" << element_name 
            //                              << "' not found in metadata. Skipping.\n";
            continue;
        }

        const element_information& info = info_iter->second;

        // Check if element should be included based on role
        bool should_include = false;

        if (info.Role == element_information::role::element) {
            // Always include chemical elements
            should_include = true;
        }
        else if (info.Role == element_information::role::isotope) {
            // Include isotopes only if requested
            should_include = include_isotopes;
        }
        // All other roles (organic_carbon, particle_size, do_not_include) are excluded

        if (should_include) {
            extracted_profile[element_name] = concentration;
        }
    }

    return extracted_profile;
}

/**
 * @brief Extract subset of elements by name
 *
 * Creates a new profile containing only the specified elements. Elements that
 * don't exist in the current profile are silently skipped. This is useful for
 * focusing analysis on specific tracers or creating subsets for comparison.
 *
 * @param element_names Vector of element names to extract
 * @return New profile containing only requested elements that exist
 *
 * @post Original profile unchanged (const method)
 * @post Result size <= element_names.size() (missing elements skipped)
 * @post Element order in result matches map order (alphabetical), not input order
 * @post Marked status and analysis inclusion flag NOT copied (future enhancement)
 *
 * @note Elements are silently skipped if they don't exist in the profile.
 *       No error or warning is generated.
 * @note The resulting profile will be in alphabetical order by element name
 *       (map ordering), not in the order specified in element_names.
 *
 * @example
 * @code
 * Elemental_Profile profile;
 * profile.AppendElement("Al", 8.5);
 * profile.AppendElement("Fe", 4.2);
 * profile.AppendElement("Pb", 25.3);
 * profile.AppendElement("Cu", 15.0);
 *
 * // Extract specific elements
 * vector<string> metals = {"Fe", "Pb", "Cu", "Zn"};  // Note: Zn doesn't exist
 * Elemental_Profile metals_only = profile.ExtractElements(metals);
 *
 * // Result contains: Fe, Pb, Cu (Zn silently skipped)
 * assert(metals_only.size() == 3);
 * assert(metals_only.Contains("Fe"));
 * assert(metals_only.Contains("Pb"));
 * assert(metals_only.Contains("Cu"));
 * assert(!metals_only.Contains("Zn"));  // Doesn't exist, skipped
 * assert(!metals_only.Contains("Al"));  // Not requested, not included
 * @endcode
 */
Elemental_Profile Elemental_Profile::ExtractElements(
    const vector<string>& element_names) const
{
    Elemental_Profile extracted_profile;

    // Reserve space if we expect most elements to exist (optimization)
    // Note: Can't reserve in map, but this shows intent

    // Iterate through requested elements
    for (const string& element_name : element_names) {
        // Check if element exists in current profile
        auto it = find(element_name);
        if (it != end()) {
            // Element exists - add to extracted profile
            extracted_profile[element_name] = it->second;
        }
        // Element doesn't exist - silently skip
    }

    return extracted_profile;
}

/**
 * @brief Get element concentration by name
 *
 * @param element_name Name of the element
 * @return Element concentration
 * @throw std::out_of_range if element does not exist
 */
double Elemental_Profile::GetValue(const string& element_name) const
{
    auto it = find(element_name);
    if (it == end()) {
        throw std::out_of_range(
            "Element '" + element_name + "' does not exist in profile"
        );
    }
    return it->second;
}


/**
 * @brief Set concentration value for existing element
 *
 * Updates the concentration of an element that already exists in the profile.
 * To add new elements, use AppendElement() instead.
 *
 * @param element_name Name of the element to update
 * @param value New concentration value
 * @return true if successful, false if element doesn't exist
 *
 * @note Does not throw exceptions - check return value
 * @note To add new elements, use AppendElement()
 *
 * @example
 * @code
 * if (profile.SetValue("Al", 10.5)) {
 *     // Successfully updated
 * } else {
 *     // Element doesn't exist
 * }
 * @endcode
 */
 bool Elemental_Profile::SetValue(const string& element_name, double value)
{
    auto it = find(element_name);
    if (it == end()) {
        return false;
    }

    it->second = value;
    return true;
}

 /**
  * @brief Set marked status for an element
  *
  * Marks or unmarks an element for quality control purposes. Marked elements
  * can be filtered in downstream analyses without removing them from the profile.
  *
  * @param element_name Name of the element
  * @param marked true to mark, false to unmark
  * @return true if successful, false if element doesn't exist
  *
  * @note Element must exist in profile before it can be marked
  */
 bool Elemental_Profile::SetMarked(const string& element_name, bool marked)
 {
     if (find(element_name) == end()) {
         return false;
     }

     marked_elements_[element_name] = marked;
     return true;
 }

 /**
  * @brief Check if element is marked
  *
  * @param element_name Name of the element
  * @return true if marked, false if unmarked or doesn't exist
  *
  * @note Returns false for both unmarked elements and non-existent elements
  */
 bool Elemental_Profile::IsMarked(const string& element_name) const
 {
     if (find(element_name) == end()) {
         return false;
     }

     auto it = marked_elements_.find(element_name);
     return (it != marked_elements_.end() && it->second);
 }


 /**
  * @brief Add new element to profile
  *
  * Adds a new element with its concentration to the profile. The element
  * must not already exist. Use SetValue() to update existing elements.
  *
  * @param element_name Name of the element (must be unique)
  * @param value Initial concentration value (default: 0.0)
  * @return true if added successfully, false if element already exists
  *
  * @post If successful: Contains(element_name) == true
  * @post If successful: GetValue(element_name) == value
  * @post If successful: IsMarked(element_name) == false
  *
  * @note To update existing elements, use SetValue() instead
  */
 bool Elemental_Profile::AppendElement(const string& element_name, double value)
 {
     if (find(element_name) != end()) {
         return false;
     }

     (*this)[element_name] = value;
     marked_elements_[element_name] = false;
     return true;
 }

 /**
  * @brief Get all concentration values as a vector
  *
  * Extracts all concentration values in map order (typically alphabetical
  * by element name). Element names are not included - use GetElementNames()
  * to get corresponding names.
  *
  * @return Vector of all concentration values
  *
  * @note Order matches iterator order of underlying map
  * @note To get element names in same order, use GetElementNames()
  */
 vector<double> Elemental_Profile::GetAllValues() const
 {
     vector<double> values;
     values.reserve(size());

     for (const auto& [element_name, concentration] : *this) {
         values.push_back(concentration);
     }

     return values;
 }

 /**
  * @brief Convert profile to human-readable string representation
  *
  * Creates a string with one element per line in "ElementName:Value" format.
  * Elements appear in map order (typically alphabetical).
  *
  * @return String representation with format "Element:Value\n" for each element
  *
  * @example
  * Output format:
  * "Al:8.5\nFe:4.2\nPb:25.3\n"
  */
 string Elemental_Profile::ToString() const
 {
     string output;

     for (const auto& [element_name, concentration] : *this) {
         output += element_name + ":" + aquiutils::numbertostring(concentration) + "\n";
     }

     return output;
 }

 /**
  * @brief Create Qt table widget representation
  *
  * Creates a QTableWidget displaying element names and concentrations.
  * Elements appear as row headers with concentrations in the first column.
  * Values outside configured limits are highlighted in red if enabled.
  *
  * @return Pointer to newly allocated QTableWidget
  *
  * @warning CALLER TAKES OWNERSHIP - must delete pointer or set Qt parent.
  *          Recommended: set a parent widget or add to layout so Qt manages lifetime.
  *
  * @note QTableWidgetItems are owned by the table and will be deleted automatically
  *
  * @example
  * @code
  * QTableWidget* table = profile.CreateTable();
  * table->setParent(mainWindow);  // Qt manages lifetime
  * // OR
  * layout->addWidget(table);      // Qt manages lifetime
  * @endcode
  */
 QTableWidget* Elemental_Profile::ToTable()
 {
     QTableWidget* table = new QTableWidget();
     table->setEditTriggers(QAbstractItemView::NoEditTriggers);
     table->setColumnCount(1);
     table->setRowCount(size());

     QStringList headers;
     QStringList element_names;

     int row = 0;
     for (const auto& [element_name, concentration] : *this) {
         element_names << QString::fromStdString(element_name);
         table->setItem(row, 0, new QTableWidgetItem(QString::number(concentration)));

         // Highlight values outside limits if enabled
         if (highlightoutsideoflimit &&
             (concentration > highlimit || concentration < lowlimit)) {
             table->item(row, 0)->setForeground(QColor(Qt::red));
         }

         row++;
     }

     headers << YAxisLabel();
     table->setHorizontalHeaderLabels(headers);
     table->setVerticalHeaderLabels(element_names);

     return table;
 }

 /**
  * @brief Serialize profile to JSON object
  *
  * Creates a JSON representation including element concentrations,
  * analysis inclusion flag, and axis labels.
  *
  * @return QJsonObject containing profile data
  *
  * @note Format: {"Include": bool, "Element Contents": {name: value, ...},
  *                "XAxisLabel": string, "YAxisLabel": string}
  */
 QJsonObject Elemental_Profile::toJsonObject() const
 {
     QJsonObject json_object;
     QJsonObject element_contents;

     json_object["Include"] = IsIncludedInAnalysis();

     for (const auto& [element_name, concentration] : *this) {
         element_contents[QString::fromStdString(element_name)] = concentration;
     }

     json_object["Element Contents"] = element_contents;
     json_object["XAxisLabel"] = XAxisLabel();
     json_object["YAxisLabel"] = YAxisLabel();

     return json_object;
 }

 /**
  * @brief Deserialize profile from JSON object
  *
  * Loads profile data from JSON, supporting two formats:
  * 1. Full format with "Include" and "Element Contents" fields
  * 2. Legacy format with elements directly in root object
  *
  * @param json_object JSON representation of profile
  * @return true on success (always succeeds currently)
  *
  * @post Profile is cleared and repopulated from JSON
  * @post Marked elements are reset (not stored in JSON)
  *
  * @note Bug fixed: YAxisLabel was being set to XAxisLabel value
  */
 bool Elemental_Profile::ReadFromJsonObject(const QJsonObject& json_object)
 {
     clear();

     if (json_object.contains("Include")) {
         // Full format with metadata
         SetIncludedInAnalysis(json_object["Include"].toBool());

         QJsonObject contents = json_object["Element Contents"].toObject();
         for (const QString& key : contents.keys()) {
             (*this)[key.toStdString()] = contents[key].toDouble();
         }
     }
     else {
         // Legacy format - elements directly in root
         for (const QString& key : json_object.keys()) {
             (*this)[key.toStdString()] = json_object[key].toDouble();
         }
     }

     if (json_object.contains("XAxisLabel")) {
         SetXAxisLabel(json_object["XAxisLabel"].toString());
     }

     if (json_object.contains("YAxisLabel")) {
         SetYAxisLabel(json_object["YAxisLabel"].toString());  // Fixed: was SetXAxisLabel
     }

     return true;
 }


 /**
  * @brief Read profile from string list
  *
  * Parses a list of strings in "ElementName:Value" format.
  * Lines without ':' separator are silently skipped.
  *
  * @param string_list List of strings with format "Element:Value"
  * @return true on success (always succeeds currently)
  *
  * @post Profile is cleared and repopulated from string list
  *
  * @example
  * Input: ["Al:8.5", "Fe:4.2", "Pb:25.3"]
  */
 bool Elemental_Profile::Read(const QStringList& string_list)
 {
     clear();

     for (const QString& line : string_list) {
         QStringList parts = line.split(":");
         if (parts.size() > 1) {
             AppendElement(parts[0].toStdString(), parts[1].toDouble());
         }
     }

     return true;
 }

 /**
  * @brief Write profile to file
  *
  * Writes the string representation of the profile to an open file.
  *
  * @param file Pointer to open QFile for writing
  * @return true on success (always succeeds currently)
  *
  * @note File must be opened for writing before calling this method
  * @note Writes in "Element:Value\n" format for each element
  */
 bool Elemental_Profile::writetofile(QFile* file)
 {
     file->write(QString::fromStdString(ToString()).toUtf8());
     return true;
 }

 /**
  * @brief Get maximum concentration value
  *
  * @return Maximum concentration across all elements
  * @return -1e12 if profile is empty (should be fixed to throw exception)
  *
  * @note Returns arbitrary minimum if profile is empty
  * @todo Consider throwing std::logic_error if empty
  */
 double Elemental_Profile::GetMaximum() const
 {
     if (empty()) {
         return -1e12;
     }

     double max_value = -1e12;
     for (const auto& [element_name, concentration] : *this) {
         if (concentration > max_value) {
             max_value = concentration;
         }
     }

     return max_value;
 }

 /**
  * @brief Get minimum concentration value
  *
  * @return Minimum concentration across all elements
  * @return 1e12 if profile is empty (should be fixed to throw exception)
  *
  * @note Returns arbitrary maximum if profile is empty
  * @todo Consider throwing std::logic_error if empty
  */
 double Elemental_Profile::GetMinimum() const
 {
     if (empty()) {
         return 1e12;
     }

     double min_value = 1e12;
     for (const auto& [element_name, concentration] : *this) {
         if (concentration < min_value) {
             min_value = concentration;
         }
     }

     return min_value;
 }

 /**
  * @brief Get list of all element names
  *
  * @return Vector of element names in map order (typically alphabetical)
  */
 vector<string> Elemental_Profile::GetElementNames() const
 {
     vector<string> names;
     names.reserve(size());

     for (const auto& [element_name, concentration] : *this) {
         names.push_back(element_name);
     }

     return names;
 }

 /**
  * @brief Check if element exists in profile
  *
  * @param element_name Name of element to check
  * @return true if element exists, false otherwise
  */
 bool Elemental_Profile::Contains(const string& element_name) const
 {
     return (find(element_name) != end());
 }


 /**
  * @brief Apply organic matter and particle size corrections
  *
  * Corrects element concentrations for organic matter content and particle
  * size effects using multiple linear regression models. Critical for ensuring
  * differences reflect source variation rather than sample characteristics.
  *
  * @param reference_om_size Vector: [organic_matter_%, mean_particle_size_um]
  * @param regression_models MLR models: element_name -> MLR(OM, size)
  * @param element_info Element metadata to identify OM and size elements
  * @return New profile with corrected concentrations
  *
  * @note Organic carbon and particle size elements are excluded from output
  * @warning Negative corrected values indicate problems with correction model
  */
 Elemental_Profile Elemental_Profile::ApplyOrganicMatterAndSizeCorrections(
     const vector<double>& reference_om_size,
     const MultipleLinearRegressionSet* regression_models,
     const map<string, element_information>* element_info) const
 {
     Elemental_Profile corrected_profile;

     for (const auto& [element_name, concentration] : *this) {
         const element_information& info = element_info->at(element_name);

         // Skip organic carbon and particle size indicators
         if (info.Role == element_information::role::organic_carbon ||
             info.Role == element_information::role::particle_size) {
             continue;
         }

         // Start with original concentration
         corrected_profile[element_name] = concentration;

         // Apply corrections if regression model exists for this element
         if (regression_models->count(element_name) == 0) {
             continue;
         }

         const MultipleLinearRegression* mlr = &regression_models->at(element_name);
         const auto& var_names = mlr->GetIndependentVariableNames();
         const auto& coefficients = mlr->CoefficientsIntercept();
         bool is_linear = (mlr->Equation() == regression_form::linear);

         // Apply first correction (typically organic matter)
         if (mlr->Effective(0) && var_names[0] != element_name) {
             double reference = reference_om_size[0];
             double measured = this->at(var_names[0]);

             if (is_linear) {
                 corrected_profile[element_name] += (reference - measured) * coefficients[1];
             }
             else {
                 corrected_profile[element_name] *= pow(reference / measured, coefficients[1]);
             }
         }

         // Apply second correction (typically particle size) if available
         if (var_names.size() > 1 && mlr->Effective(1) && var_names[1] != element_name) {
             double reference = reference_om_size[1];
             double measured = this->at(var_names[1]);

             if (is_linear) {
                 corrected_profile[element_name] += (reference - measured) * coefficients[2];
             }
             else {
                 corrected_profile[element_name] *= pow(reference / measured, coefficients[2]);
             }
         }

         // Warn about negative corrected values (except for isotopes)
         if (corrected_profile[element_name] < 0 &&
             info.Role != element_information::role::isotope) {
             std::cerr << "Warning: Corrected value for " << element_name
                 << " is negative: " << corrected_profile[element_name] << std::endl;
         }
     }

     return corrected_profile;
 }


 /**
  * @brief Calculate dot product with weight vector
  *
  * Computes sum(concentration[i] * weights[i]) for all elements.
  * Used in chemical mass balance and discriminant analysis calculations.
  *
  * @param weights Weight vector (must match profile size)
  * @return Dot product value, or -999 if size mismatch
  *
  * @warning Returns -999 for size mismatch (should throw exception instead)
  * @note Element order matches map iterator order (alphabetical)
  *
  * @todo Replace magic number with exception for size mismatch
  */
 double Elemental_Profile::CalculateDotProduct(const CMBVector& weights) const
 {
     if (size() != weights.getsize()) {
         return -999;
     }

     double sum = 0.0;
     int index = 0;

     for (const auto& [element_name, concentration] : *this) {
         sum += concentration * weights[index];
         index++;
     }

     return sum;
 }

 /**
  * @brief Sort elements by concentration value
  *
  * Creates a CMBVector containing elements sorted by their concentration.
  * Uses selection sort algorithm (O(n²) - consider std::sort for large profiles).
  *
  * @param ascending true for ascending order (low to high), false for descending
  * @return CMBVector with sorted element names and values
  *
  * @note Original profile unchanged (const method)
  * @note Performance: O(n²) - fine for typical profile sizes (<100 elements)
  */
 CMBVector Elemental_Profile::SortByConcentration(bool ascending) const
 {
     CMBVector sorted_result;
     vector<string> processed_elements;

     for (size_t i = 0; i < size(); i++) {
         double target_value = ascending ? 1e6 : -1e6;
         string target_element;

         for (const auto& [element_name, concentration] : *this) {
             // Skip already processed elements
             if (lookup(processed_elements, element_name) != -1) {
                 continue;
             }

             // Find min (ascending) or max (descending)
             bool is_better = ascending ? (concentration < target_value)
                 : (concentration > target_value);

             if (is_better) {
                 target_element = element_name;
                 target_value = concentration;
             }
         }

         processed_elements.push_back(target_element);
         sorted_result.append(target_element, target_value);
     }

     return sorted_result;
 }

 /**
  * @brief Select top N elements by concentration
  *
  * Returns the N highest (or lowest) concentrated elements.
  * Useful for focusing analysis on dominant tracers.
  *
  * @param n Number of elements to select
  * @param ascending true to select lowest values, false for highest
  * @return Vector of element names (up to N elements)
  *
  * @post Return size <= min(n, profile.size())
  */
 vector<string> Elemental_Profile::SelectTopElements(int n, bool ascending) const
 {
     CMBVector sorted = SortByConcentration(ascending);
     vector<string> top_elements;

     for (int i = 0; i < n && i < sorted.num; i++) {
         top_elements.push_back(sorted.Label(i));
     }

     return top_elements;
 }
