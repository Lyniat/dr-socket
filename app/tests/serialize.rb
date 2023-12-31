# https://stackoverflow.com/questions/30652959/rails-display-difference-between-two-hashes
def hash_diff(a, b)
    a.reject { |k, v| b[k] == v }.merge!(b.reject { |k, _v| a.key?(k) })
end

def get_test_data
    test_data = {
        "this_is_data?" => true,
        :letters => [
            "a",
            "b",
            "c",
            "d",
        ],
        "numbers" => [
            [1, 1.1],
            [2, 2.2],
            [-3, -3.3],
        ],
        "random_string_in_between": "--",
        "dev_info" => {
            "gender" => "raccoon",
            :age => 256,
            "name" => "lyniat",
            "knows_how_to_code" => true,
            "knows_how_to_code_good" => false,
            :credit_card => {
                "type" => "Coders Club",
                "card number" => "0016 0032 0064 0128",
            },
        },
        "what_causes_exceptions?": nil,
        "different_keys": {
            567 => "567",
            "abc" => "ABC",
            3.14 => "PI",
        },
        "end_of_data" => true,
    }
end

def test_serialize args, assert
    test_data = get_test_data
    data = __test_serialize(test_data)
    result = __test_deserialize(data)
    diff = hash_diff(test_data, result)

    assert.true! diff.empty?, "Test hashes differ! #{diff}"
end