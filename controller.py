from Util import generate_csv_with_out_heads,get_file_content
import subprocess


inputVariablesDict  = {
    "lactation_cow":3,
    "monthAfterCalving_cow":5,
    "monthInPregnancy_cow":1,
    "milkProduction_cow":85,
    "expectedProductionRestOfLactation_cow":100,
    "expectedProductionNextLactation_cow":100,
    "additionalMilkReplacement_cow":0,
    "herdTurnoverRatio_cow":35,
    "rha_cow":24000,
    "pregnancyRate_21_D_cow":20,
    "reproCost_cow":20,
    "lastDimToBreed_cow":10,
    "milkThresholdToCull_cow":50,
    "between35TO250DaysInPregnancy_cow":22.6,
    "avgCowBodyWeight_cow":1306,
    "replacementValue_cow":1300,
    "salvageValue_cow":0.38,
    "calfValue_cow":100,
    "milkPrice_cow":15.88,
    "milkButterfat_cow":3.5,
    "feedPriceProduction_cow":0.1,
    "feedPriceDry_cow":0.08,
    "interestRate_cow":6,
    "conceptionRate_CS":60,
    "service_SS":0,
    "female_CS":47,
    "female_SS":90,
    "Calf_Mortality":7,
    "avg_heifer_service_rate":75
}

def create_input_file(inputVariablesDict):

    inputContent = [
        inputVariablesDict["lactation_cow"],
        inputVariablesDict["monthAfterCalving_cow"],
        inputVariablesDict["monthInPregnancy_cow"],
        inputVariablesDict["milkProduction_cow"],
        inputVariablesDict["expectedProductionRestOfLactation_cow"],
        inputVariablesDict["expectedProductionNextLactation_cow"],
        inputVariablesDict["additionalMilkReplacement_cow"],
        inputVariablesDict["herdTurnoverRatio_cow"],
        inputVariablesDict["rha_cow"],
        inputVariablesDict["pregnancyRate_21_D_cow"],
        inputVariablesDict["reproCost_cow"],
        inputVariablesDict["lastDimToBreed_cow"],
        inputVariablesDict["milkThresholdToCull_cow"],
        inputVariablesDict["between35TO250DaysInPregnancy_cow"],
        inputVariablesDict["avgCowBodyWeight_cow"],
        inputVariablesDict["replacementValue_cow"],
        inputVariablesDict["salvageValue_cow"],
        inputVariablesDict["calfValue_cow"],
        inputVariablesDict["milkPrice_cow"],
        inputVariablesDict["milkButterfat_cow"],
        inputVariablesDict["feedPriceProduction_cow"],
        inputVariablesDict["feedPriceDry_cow"],
        inputVariablesDict["interestRate_cow"],
        #inputVariablesDict["conceptionRate_CS"],
        #inputVariablesDict["service_SS"],
        #inputVariablesDict["female_CS"],
        #inputVariablesDict["female_SS"],
        #inputVariablesDict["Calf_Mortality"],
        #inputVariablesDict["avg_heifer_service_rate"]
    ]

    generate_csv_with_out_heads(inputContent,'./input/input.csv')

def run_calculate_cow_c_code():

    command = ['./c/calculate', '-n', '1', '-i', './input/input.csv']
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode == 0:
        return True
    else:
        return False

def get_output_content():
    return get_file_content('./output/output.csv')

create_input_file(inputVariablesDict)
run_calculate_cow_c_code()
outut = get_output_content()

print(outut)